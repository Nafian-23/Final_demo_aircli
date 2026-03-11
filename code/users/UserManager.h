#ifndef USERS_USERMANAGER_H
#define USERS_USERMANAGER_H

#include <map>
#include <vector>
#include "User.h"

using namespace std;

namespace users {
    
    class UserManager {
    private:
        map<string, User> usersById;
        map<string, string> usernameToId;

    public:

        bool load();
        
        bool save() const;
        
        string createUser(const string &name, const string &username, 
                   const string &password, const string &phoneNumber);
        
        // Get a user by their ID
        const User* get(const string &id) const;
        
        // Find a user by their username
        const User* findByUsername(const string &username) const;

        // Find a user by phone number
        const User* findByPhoneNumber(const string &phoneNumber) const;
        
        // Verify credentials and return user ID if valid
        bool authenticate(const string &username, const string &password, 
                         string &outId) const;
        
        // Get all users
        vector<User> all() const;
        
        // Update a user's role
        bool updateRole(const string &id, Role newRole);

        // Update a user's cash balance
        bool updateCashBalance(const string &id, long long amount);

        // Update a user's cash balance by phone
        bool updateCashBalanceByPhone(const string &phoneNumber, long long amount);

        // Update a user's phone number
        bool updatePhoneNumber(const string &id, const string &phoneNumber);

        // Update bank link (bank user id)
        bool updateLinkedBankUserId(const string &id, const string &bankUserId);

        // Update realistic bank link (bank + account)
        bool updateLinkedBankAccount(const string &id, const string &bankId, const string &accountNumber);
        bool clearLinkedBankAccount(const string &id);

        // Credit cash only when bank withdrawal is verified
        bool creditCashForLinkedBankAccount(const string &bankId, const string &accountNumber, long long amountCents);
    };
}

#endif
