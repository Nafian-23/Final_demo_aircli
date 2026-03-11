#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <vector>
#include <termios.h>
#include <unistd.h>

#include "storage/DataStorage.h"
#include "banking/BankDirectory.h"
#include "users/UserManager.h"

using namespace std;

// Reads a line with characters hidden (shows '*' instead). Uses termios to disable echo.
static string readMaskedPassword() {
    if (!isatty(STDIN_FILENO)) {
        string s;
        getline(cin, s);
        return s;
    }
    termios oldAttr, newAttr;
    tcgetattr(STDIN_FILENO, &oldAttr);
    newAttr = oldAttr;
    newAttr.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newAttr);
    string password;
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != '\n') {
        password += c;
        cout << '*';
        cout.flush();
    }
    cout << '\n';
    tcsetattr(STDIN_FILENO, TCSANOW, &oldAttr);
    return password;
}

struct BankCustomer {
    string id;
    string name;
    string phone;
    string pin;
};

struct BankAccount {
    string accountNumber;
    string bankId;
    string customerId;
    long long balanceCents;
    string status;
};

enum class BankTxType { Credit = 0, Debit = 1 };

static vector<string> splitPipe(const string &line) {
    vector<string> parts;
    string part;
    stringstream ss(line);
    while (getline(ss, part, '|')) parts.push_back(part);
    return parts;
}

static long long nowSec() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

static string newId(const string &prefix) {
    long long t = nowSec();
    return prefix + to_string(t);
}

static long long toCents(long long amount) { return amount * 100; }

static void printFormattedBalance(long long cents) {
    long long taka = cents / 100;
    long long paisa = cents % 100;
    cout << "BDT " << taka << "." << setfill('0') << setw(2) << paisa;
}

static vector<BankCustomer> loadCustomers() {
    vector<BankCustomer> customers;
    auto lines = storage::DataStorage::readAll("bank_customers.txt");
    for (const auto &l : lines) {
        if (l.empty()) continue;
        auto p = splitPipe(l);
        if (p.size() < 4) continue;
        customers.push_back(BankCustomer{p[0], p[1], p[2], p[3]});
    }
    return customers;
}

static bool saveCustomers(const vector<BankCustomer> &customers) {
    vector<string> lines;
    lines.reserve(customers.size());
    for (const auto &c : customers) {
        lines.push_back(c.id + "|" + c.name + "|" + c.phone + "|" + c.pin);
    }
    return storage::DataStorage::writeAll("bank_customers.txt", lines);
}

static const BankCustomer* findCustomerById(const vector<BankCustomer> &customers, const string &id) {
    for (const auto &c : customers) if (c.id == id) return &c;
    return nullptr;
}

static const BankCustomer* findCustomerByPhone(const vector<BankCustomer> &customers, const string &phone) {
    for (const auto &c : customers) if (c.phone == phone) return &c;
    return nullptr;
}

static vector<BankAccount> loadAccounts() {
    vector<BankAccount> accounts;
    auto lines = storage::DataStorage::readAll("bank_accounts.txt");
    for (const auto &l : lines) {
        if (l.empty()) continue;
        auto p = splitPipe(l);
        if (p.size() < 5) continue;
        long long bal = 0;
        try { bal = stoll(p[3]); } catch (...) { bal = 0; }
        accounts.push_back(BankAccount{p[0], p[1], p[2], bal, p[4]});
    }
    return accounts;
}

static bool saveAccounts(const vector<BankAccount> &accounts) {
    vector<string> lines;
    lines.reserve(accounts.size());
    for (const auto &a : accounts) {
        lines.push_back(a.accountNumber + "|" + a.bankId + "|" + a.customerId + "|" + to_string(a.balanceCents) + "|" + a.status);
    }
    return storage::DataStorage::writeAll("bank_accounts.txt", lines);
}

static BankAccount* findAccount(vector<BankAccount> &accounts, const string &bankId, const string &accountNumber) {
    for (auto &a : accounts) {
        if (a.bankId == bankId && a.accountNumber == accountNumber) return &a;
    }
    return nullptr;
}

static vector<BankAccount> accountsForCustomer(const vector<BankAccount> &accounts, const string &customerId) {
    vector<BankAccount> out;
    for (const auto &a : accounts) if (a.customerId == customerId) out.push_back(a);
    return out;
}

static void appendTx(const string &bankId, const string &accountNumber, long long amountCents, BankTxType type, const string &desc) {
    // bank_tx.txt: id|bankId|accountNumber|amountCents|type|desc|timestampSec
    const string id = newId("T");
    const long long ts = nowSec();
    const string line =
        id + "|" + bankId + "|" + accountNumber + "|" + to_string(amountCents) + "|" +
        to_string((int)type) + "|" + desc + "|" + to_string(ts);
    storage::DataStorage::appendLine("bank_tx.txt", line);
}

static bool bankWelcome(string &outCustomerId) {
    banking::BankDirectory::ensureSeedBanks();
    while (true) {
        cout << "\n==================================================\n";
        cout << "                 BankCLI 🏦\n";
        cout << "==================================================\n\n";
        cout << "1. Login\n";
        cout << "2. Register (Create Customer)\n";
        cout << "3. Exit\n\n";
        cout << "What would you like to do? > ";

        int choice = 0;
        cin >> choice;
        cin.ignore();

        if (choice == 1) {
            cout << "\n--- Bank Login ---\n";
            cout << "Phone Number : ";
            string phone;
            getline(cin, phone);
            cout << "PIN          : ";
            cout.flush();
            string pin = readMaskedPassword();

            auto customers = loadCustomers();
            const auto *c = findCustomerByPhone(customers, phone);
            if (c && c->pin == pin) {
                outCustomerId = c->id;
                cout << "\n✓ Login successful. Welcome, " << c->name << "!\n";
                return true;
            }
            cout << "\nIncorrect phone or PIN.\n";
        } else if (choice == 2) {
            cout << "\n--- Register Customer ---\n";
            cout << "Full Name    : ";
            string name;
            getline(cin, name);
            cout << "Phone Number : ";
            string phone;
            getline(cin, phone);

            auto customers = loadCustomers();
            if (findCustomerByPhone(customers, phone)) {
                cout << "\nThis phone number is already registered.\n";
                continue;
            }

            cout << "PIN          : ";
            cout.flush();
            string pin = readMaskedPassword();
            cout << "Confirm PIN  : ";
            cout.flush();
            string confirmPin = readMaskedPassword();
            if (pin != confirmPin) {
                cout << "\nPINs don't match.\n";
                continue;
            }

            const string id = newId("C");
            customers.push_back(BankCustomer{id, name, phone, pin});
            saveCustomers(customers);
            outCustomerId = id;
            cout << "\n✓ Customer created. You can now open a bank account.\n";
            return true;
        } else if (choice == 3) {
            cout << "\nThanks for using BankCLI. Goodbye!\n";
            return false;
        } else {
            cout << "\nPlease enter 1, 2, or 3.\n";
        }
    }
}

static bool chooseAccount(const string &customerId, string &outBankId, string &outAccountNumber) {
    auto accounts = loadAccounts();
    auto mine = accountsForCustomer(accounts, customerId);
    auto banks = banking::BankDirectory::listBanks();

    cout << "\n--- Select Account ---\n";
    if (!mine.empty()) {
        cout << "Your accounts:\n";
        for (size_t i = 0; i < mine.size(); i++) {
            string bankName;
            for (const auto &b : banks) if (b.id == mine[i].bankId) bankName = b.name;
            cout << "  " << (i + 1) << ") " << bankName << " | " << mine[i].accountNumber << "\n";
        }
        cout << "  " << (mine.size() + 1) << ") Open a new account\n";
        cout << "Choice: ";
        int choice = 0;
        cin >> choice;
        cin.ignore();
        if (choice >= 1 && choice <= (int)mine.size()) {
            outBankId = mine[(size_t)choice - 1].bankId;
            outAccountNumber = mine[(size_t)choice - 1].accountNumber;
            return true;
        }
        if (choice != (int)mine.size() + 1) {
            cout << "\nInvalid choice.\n";
            return false;
        }
    } else {
        cout << "You have no bank accounts yet. Let's open one.\n";
    }

    cout << "\nSelect a bank:\n";
    for (size_t i = 0; i < banks.size(); i++) {
        cout << "  " << (i + 1) << ") " << banks[i].name << " (" << banks[i].id << ")\n";
    }
    cout << "Choice: ";
    int bankChoice = 0;
    cin >> bankChoice;
    cin.ignore();
    if (bankChoice < 1 || bankChoice > (int)banks.size()) {
        cout << "\nInvalid bank choice.\n";
        return false;
    }
    const string bankId = banks[(size_t)bankChoice - 1].id;

    // Create a new account number
    const string accountNumber = "AC" + to_string(nowSec());
    accounts.push_back(BankAccount{accountNumber, bankId, customerId, 0, "ACTIVE"});
    saveAccounts(accounts);

    cout << "\n✓ New account opened: " << accountNumber << "\n";
    outBankId = bankId;
    outAccountNumber = accountNumber;
    return true;
}

int main() {
    users::UserManager userManager;
    userManager.load();

    string customerId;
    if (!bankWelcome(customerId)) return 0;

    auto customers = loadCustomers();
    const auto *me = findCustomerById(customers, customerId);
    const string myPhone = me ? me->phone : "";

    string bankId;
    string accountNumber;
    if (!chooseAccount(customerId, bankId, accountNumber)) return 0;

    bool isRunning = true;
    while (isRunning) {
        auto accounts = loadAccounts();
        userManager.load();
        BankAccount *acc = findAccount(accounts, bankId, accountNumber);
        const long long bal = acc ? acc->balanceCents : 0;

        cout << "\n==============================\n";
        cout << " Banking Services\n";
        cout << "==============================\n";
        cout << "\nActive Account: " << accountNumber << " (" << bankId << ")\n";
        cout << "Bank Balance : ";
        printFormattedBalance(bal);

        cout << "\n\n1. Switch Account / Open New\n";
        cout << "2. Deposit Funds\n";
        cout << "3. Withdraw Funds (to airCLI Cash)\n";
        cout << "4. Exit Banking\n";
        cout << "\nSelect an option > ";

        int choice = 0;
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1: {
            if (!chooseAccount(customerId, bankId, accountNumber)) {
                cout << "\nCould not select account.\n";
            }
            break;
        }
        case 2: {
            cout << "\n--- Deposit Funds ---\n";
            cout << "Enter amount to deposit (BDT): ";
            long long amount;
            cin >> amount;
            cin.ignore();
            if (amount <= 0) {
                cout << "\nInvalid amount.\n";
                break;
            }
            if (!acc) {
                cout << "\nAccount not found.\n";
                break;
            }
            acc->balanceCents += toCents(amount);
            saveAccounts(accounts);
            appendTx(bankId, accountNumber, toCents(amount), BankTxType::Credit, "Deposit");
            cout << "\n✓ Deposit successful.\n";
            break;
        }
        case 3: {
            cout << "\n--- Withdraw Funds ---\n";
            cout << "Enter amount to withdraw (BDT): ";
            long long amount;
            cin >> amount;
            cin.ignore();
            if (amount <= 0) {
                cout << "\nInvalid amount.\n";
                break;
            }
            if (!acc) {
                cout << "\nAccount not found.\n";
                break;
            }
            const long long cents = toCents(amount);
            if (cents > acc->balanceCents) {
                cout << "\nInsufficient bank balance.\n";
                break;
            }
            acc->balanceCents -= cents;
            saveAccounts(accounts);
            appendTx(bankId, accountNumber, cents, BankTxType::Debit, "Withdrawal to airCLI cash");

            const bool credited = userManager.creditCashForLinkedBankAccount(bankId, accountNumber, cents);
            if (!credited) {
                cout << "\nWithdrawal done, but no linked airCLI account was found.\n";
                cout << "Link this bank account in airCLI first to receive cash.\n";
            } else {
                cout << "\n✓ Withdrawal successful. Cash credited to linked airCLI user.\n";
            }
            (void)myPhone;
            break;
        }
        case 4:
            cout << "\nExiting Banking CLI. Goodbye!\n";
            isRunning = false;
            break;
        default:
            cout << "\nInvalid option.\n";
        }
    }

    return 0;
}
