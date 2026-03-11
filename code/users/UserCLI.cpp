#include "UserCLI.h"
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include "../banking/BankDirectory.h"

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

namespace users {
    void UserCLI::showWelcome() {
        cout << "\n==================================================\n";
        cout << "              Welcome to airCLI ✈\n";
        cout << "              Flight Booking System\n";
        cout << "==================================================\n\n";
        cout << "1. Login\n";
        cout << "2. Create New Account\n";
        cout << "3. Exit\n\n";
        cout << "What would you like to do? > ";
    }

    bool UserCLI::runSignup(string &outUserId) {
        cout << "\n--- Create Your Account ---\n";
        
        cout << "\nFull Name     : ";
        string name;
        cin.ignore();
        getline(cin, name);
        
        cout << "Username      : ";
        string username;
        getline(cin, username);
        
        if (manager.findByUsername(username)) {
            cout << "\nSorry, that username is already taken. Please try another.\n";
            return false;
        }
        
        cout << "Password      : ";
        cout.flush();
        string password = readMaskedPassword();
        
        cout << "Confirm       : ";
        cout.flush();
        string confirmPassword = readMaskedPassword();
        
        if (password != confirmPassword) {
            cout << "\nThe passwords don't match. Please try again.\n";
            return false;
        }
        
        cout << "Phone Number  : ";
        string phoneNumber;
        getline(cin, phoneNumber);

        outUserId = manager.createUser(name, username, password, phoneNumber);
        
        cout << "\n✓ Account created successfully!\n";
        cout << "Your user ID: " << outUserId << "\n";
        cout << "You can now log in with your username and password.\n";
        
        return true;
    }

    bool UserCLI::runLogin(string &outUserId) {
        cout << "\n--- Login ---\n";
        
        cout << "\nUsername : ";
        string username;
        cin.ignore();
        getline(cin, username);
        
        cout << "Password : ";
        cout.flush();
        string password = readMaskedPassword();
        
        if (!manager.authenticate(username, password, outUserId)) {
            cout << "\nIncorrect username or password. Please try again.\n";
            return false;
        }
        
        const User* user = manager.get(outUserId);
        cout << "\n✓ Login successful!\n";
        cout << "Welcome back, " << user->getName() << "!\n";
        
        return true;
    }

    bool UserCLI::linkBankAccount(const string &userId) {
        manager.load();
        const User* user = manager.get(userId);
        if (!user) {
            cout << "\nUser not found.\n";
            return false;
        }

        if (!user->getLinkedBankAccountNumber().empty()) {
            cout << "\nBank account already added. Want to unlink? (y/n): ";
            char choice = 'n';
            cin >> choice;
            if (choice == 'y' || choice == 'Y') {
                manager.clearLinkedBankAccount(userId);
                cout << "\nBank account unlinked.\n";
                return false;
            }

            cout << "\nOpen Banking CLI? (y/n): ";
            cin >> choice;
            return (choice == 'y' || choice == 'Y');
        }

        cout << "\n--- Link Bank Account ---\n";
        auto banks = banking::BankDirectory::listBanks();
        if (banks.empty()) {
            cout << "\nNo banks available.\n";
            return false;
        }

        cout << "\nSelect a bank:\n";
        for (size_t i = 0; i < banks.size(); i++) {
            cout << "  " << (i + 1) << ") " << banks[i].name << " (" << banks[i].id << ")\n";
        }
        cout << "Choice: ";
        int bankChoice = 0;
        cin >> bankChoice;
        if (bankChoice < 1 || bankChoice > (int)banks.size()) {
            cout << "\nInvalid bank choice.\n";
            return false;
        }
        const string bankId = banks[(size_t)bankChoice - 1].id;

        cout << "Account number: ";
        string accountNumber;
        cin.ignore();
        getline(cin, accountNumber);

        cout << "Bank account phone: ";
        string phone;
        getline(cin, phone);

        cout << "Bank PIN: ";
        cout.flush();
        string pin = readMaskedPassword();

        if (!user->getPhoneNumber().empty() && user->getPhoneNumber() != phone) {
            cout << "\nPhone number does not match your airCLI profile.\n";
            cout << "Your profile phone: " << user->getPhoneNumber() << "\n";
            return false;
        }

        if (user->getPhoneNumber().empty()) {
            manager.updatePhoneNumber(userId, phone);
        }

        banking::VerifiedBankAccount verified;
        Result vr = banking::BankDirectory::verifyAccountCredentials(
            bankId, accountNumber, phone, pin, verified
        );
        if (!vr.ok) {
            cout << "\n" << vr.message << "\n";
            cout << "Open Banking CLI now to create/verify account? (y/n): ";
            char openChoice = 'n';
            cin >> openChoice;
            return (openChoice == 'y' || openChoice == 'Y');
        }

        manager.updateLinkedBankAccount(userId, bankId, accountNumber);
        cout << "\n✓ Bank account linked successfully (" << verified.bankName << ").\n";
        cout << "Open Banking CLI now? (y/n): ";
        char openChoice = 'n';
        cin >> openChoice;
        return (openChoice == 'y' || openChoice == 'Y');
    }

    bool UserCLI::welcomeFlow(string &outUserId, Role &outRole) {
        manager.load();
        
        while (true) {
            showWelcome();
            int choice = 0;
            cin >> choice;
            
            if (choice == 1) {
                if (runLogin(outUserId)) {
                    const User* user = manager.get(outUserId);
                    outRole = user->getRole();
                    return true;
                }
            } else if (choice == 2) {
                if (runSignup(outUserId)) {
                    const User* user = manager.get(outUserId);
                    outRole = user->getRole();
                    return true;
                }
            } else if (choice == 3) {
                cout << "\nThanks for visiting airCLI. Goodbye!\n";
                return false;
            } else {
                cout << "\nPlease enter 1, 2, or 3.\n";
            }
        }
    }
}
