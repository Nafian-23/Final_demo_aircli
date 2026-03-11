#include "BankDirectory.h"

#include <sstream>
#include <chrono>

#include "../storage/DataStorage.h"

using namespace std;

namespace banking {

    static vector<string> splitPipe(const string &line) {
        vector<string> parts;
        string part;
        stringstream ss(line);
        while (getline(ss, part, '|')) parts.push_back(part);
        return parts;
    }

    void BankDirectory::ensureSeedBanks() {
        auto lines = storage::DataStorage::readAll("banks.txt");
        if (!lines.empty()) return;

        vector<string> seed = {
            "B001|Prime Bank",
            "B002|City Bank",
            "B003|Dutch-Bangla Bank",
            "B004|BRAC Bank",
            "B005|Islami Bank"
        };
        storage::DataStorage::writeAll("banks.txt", seed);
    }

    vector<Bank> BankDirectory::listBanks() {
        ensureSeedBanks();
        vector<Bank> banks;
        auto lines = storage::DataStorage::readAll("banks.txt");
        for (const auto &l : lines) {
            if (l.empty()) continue;
            auto p = splitPipe(l);
            if (p.size() < 2) continue;
            banks.push_back(Bank{p[0], p[1]});
        }
        return banks;
    }

    Result BankDirectory::verifyAccountCredentials(
        const string &bankId,
        const string &accountNumber,
        const string &phone,
        const string &pin,
        VerifiedBankAccount &outVerified
    ) {
        ensureSeedBanks();

        string bankName;
        {
            auto banks = storage::DataStorage::readAll("banks.txt");
            for (const auto &l : banks) {
                auto p = splitPipe(l);
                if (p.size() >= 2 && p[0] == bankId) {
                    bankName = p[1];
                    break;
                }
            }
        }
        if (bankName.empty()) return Result::Err("Bank not found.");

        // bank_accounts.txt: accountNumber|bankId|customerId|balanceCents|status
        string customerId;
        {
            auto accounts = storage::DataStorage::readAll("bank_accounts.txt");
            for (const auto &l : accounts) {
                if (l.empty()) continue;
                auto p = splitPipe(l);
                if (p.size() < 5) continue;
                if (p[0] == accountNumber && p[1] == bankId) {
                    customerId = p[2];
                    break;
                }
            }
        }
        if (customerId.empty()) return Result::Err("Account not found for that bank.");

        // bank_customers.txt: customerId|fullName|phone|pin
        string customerName;
        string storedPhone;
        string storedPin;
        {
            auto customers = storage::DataStorage::readAll("bank_customers.txt");
            for (const auto &l : customers) {
                if (l.empty()) continue;
                auto p = splitPipe(l);
                if (p.size() < 4) continue;
                if (p[0] == customerId) {
                    customerName = p[1];
                    storedPhone = p[2];
                    storedPin = p[3];
                    break;
                }
            }
        }
        if (customerName.empty()) return Result::Err("Bank customer record not found.");

        if (storedPhone != phone) return Result::Err("Phone number does not match bank record.");
        if (storedPin != pin) return Result::Err("Invalid PIN.");

        outVerified = VerifiedBankAccount{
            bankId,
            bankName,
            accountNumber,
            customerId,
            customerName,
            storedPhone
        };
        return Result::Ok();
    }

    static long long nowSec() {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

    static void appendBankTx(const string &bankId, const string &accountNumber, long long amountCents, int type, const string &desc) {
        // bank_tx.txt: id|bankId|accountNumber|amountCents|type|desc|timestampSec
        const string id = "T" + to_string(nowSec());
        const string line =
            id + "|" + bankId + "|" + accountNumber + "|" + to_string(amountCents) + "|" +
            to_string(type) + "|" + desc + "|" + to_string(nowSec());
        storage::DataStorage::appendLine("bank_tx.txt", line);
    }

    Result BankDirectory::debitVerifiedAccount(
        const string &bankId,
        const string &accountNumber,
        const string &phone,
        const string &pin,
        long long amountCents,
        const string &description
    ) {
        if (amountCents <= 0) return Result::Err("Invalid amount.");

        VerifiedBankAccount verified;
        Result vr = verifyAccountCredentials(bankId, accountNumber, phone, pin, verified);
        if (!vr.ok) return vr;

        // bank_accounts.txt: accountNumber|bankId|customerId|balanceCents|status
        auto lines = storage::DataStorage::readAll("bank_accounts.txt");
        bool found = false;
        vector<string> updated;
        updated.reserve(lines.size() + 1);

        for (const auto &l : lines) {
            if (l.empty()) continue;
            auto p = splitPipe(l);
            if (p.size() < 5) continue;

            if (p[0] == accountNumber && p[1] == bankId) {
                found = true;
                long long bal = 0;
                try { bal = stoll(p[3]); } catch (...) { bal = 0; }
                if (bal < amountCents) return Result::Err("Insufficient bank balance.");
                bal -= amountCents;
                p[3] = to_string(bal);
                updated.push_back(p[0] + "|" + p[1] + "|" + p[2] + "|" + p[3] + "|" + p[4]);
            } else {
                updated.push_back(l);
            }
        }

        if (!found) return Result::Err("Account not found.");
        storage::DataStorage::writeAll("bank_accounts.txt", updated);
        appendBankTx(bankId, accountNumber, amountCents, 1, description);
        return Result::Ok();
    }

    Result BankDirectory::debitByAccountAndPin(
        const string &accountNumber,
        const string &pin,
        long long amountCents,
        const string &description
    ) {
        if (amountCents <= 0) return Result::Err("Invalid amount.");
        if (accountNumber.empty()) return Result::Err("Account number required.");
        if (pin.empty()) return Result::Err("PIN required.");

        // Find account by number (accountNumber|bankId|customerId|balanceCents|status)
        string bankId;
        string customerId;
        long long balance = 0;
        string status;

        auto lines = storage::DataStorage::readAll("bank_accounts.txt");
        for (const auto &l : lines) {
            if (l.empty()) continue;
            auto p = splitPipe(l);
            if (p.size() < 5) continue;
            if (p[0] == accountNumber) {
                bankId = p[1];
                customerId = p[2];
                try { balance = stoll(p[3]); } catch (...) { balance = 0; }
                status = p[4];
                break;
            }
        }
        if (bankId.empty() || customerId.empty()) return Result::Err("Account not found.");
        if (status != "ACTIVE") return Result::Err("Account is not active.");

        // Verify PIN for that customer (customerId|fullName|phone|pin)
        string storedPin;
        {
            auto customers = storage::DataStorage::readAll("bank_customers.txt");
            for (const auto &l : customers) {
                if (l.empty()) continue;
                auto p = splitPipe(l);
                if (p.size() < 4) continue;
                if (p[0] == customerId) {
                    storedPin = p[3];
                    break;
                }
            }
        }
        if (storedPin.empty()) return Result::Err("Bank customer record not found.");
        if (storedPin != pin) return Result::Err("Account number / PIN mismatch.");

        if (balance < amountCents) return Result::Err("Insufficient bank balance.");

        // Debit and persist
        vector<string> updated;
        updated.reserve(lines.size());
        for (const auto &l : lines) {
            if (l.empty()) continue;
            auto p = splitPipe(l);
            if (p.size() < 5) continue;
            if (p[0] == accountNumber && p[1] == bankId) {
                long long bal = 0;
                try { bal = stoll(p[3]); } catch (...) { bal = 0; }
                bal -= amountCents;
                p[3] = to_string(bal);
                updated.push_back(p[0] + "|" + p[1] + "|" + p[2] + "|" + p[3] + "|" + p[4]);
            } else {
                updated.push_back(l);
            }
        }
        storage::DataStorage::writeAll("bank_accounts.txt", updated);
        appendBankTx(bankId, accountNumber, amountCents, 1, description);
        return Result::Ok();
    }
}

