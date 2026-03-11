#ifndef USERS_USER_H
#define USERS_USER_H

#include "../Role.h"

#include <string>

using namespace std;

namespace users {
    
    class User {
    private:
        string id;
        string name;
        string username;
        string password;
        Role role;
        long long cashBalance; // Cash at hand from bank withdrawals
        string phoneNumber;
        // Legacy field from older version (kept for backward-compatible deserialization).
        string linkedBankUserId;
        // New realistic linkage: bank + account number.
        string linkedBankBankId;
        string linkedBankAccountNumber;

    public:

                User() : role(Role::Passenger), cashBalance(0) {}

                User(const string &userId, const string &fullName, 
                         const string &userName, const string &pass, Role userRole)
                        : id(userId), name(fullName), username(userName), 
                            password(pass), role(userRole), cashBalance(0) {}

        // Getters
        const string &getId() const { return id; }
        const string &getName() const { return name; }
        const string &getUsername() const { return username; }
        const string &getPassword() const { return password; }
        
        Role getRole() const { return role; }

        long long getCashBalance() const { return cashBalance; }
        void setCashBalance(long long amount) { cashBalance = amount; }
        void addCash(long long amount) { cashBalance += amount; }
        void subtractCash(long long amount) { if (cashBalance >= amount) cashBalance -= amount; }

        const string &getPhoneNumber() const { return phoneNumber; }
        void setPhoneNumber(const string &phone) { phoneNumber = phone; }

        const string &getLinkedBankUserId() const { return linkedBankUserId; }
        void setLinkedBankUserId(const string &bankUserId) { linkedBankUserId = bankUserId; }

        const string &getLinkedBankBankId() const { return linkedBankBankId; }
        const string &getLinkedBankAccountNumber() const { return linkedBankAccountNumber; }
        void setLinkedBankAccount(const string &bankId, const string &accountNumber) {
            linkedBankBankId = bankId;
            linkedBankAccountNumber = accountNumber;
        }
        void clearLinkedBankAccount() {
            linkedBankBankId.clear();
            linkedBankAccountNumber.clear();
        }

        // Update user role
        void setRole(Role newRole) { role = newRole; }

        // Storage Functions
        string serialize() const;
        static User deserialize(const string &line);
    };
}

#endif
