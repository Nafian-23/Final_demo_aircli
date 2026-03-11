#include "User.h"
#include <sstream>
#include <vector>

using namespace std;

namespace users {
    string User::serialize() const {

        // Format:
        // id|name|username|password|role|cashBalance|phone|linkedBankUserId|linkedBankBankId|linkedBankAccountNumber
        return id + "|" + name + "|" + username + "|" + password + 
               "|" + to_string(static_cast<int>(role)) + "|" + to_string(cashBalance) +
               "|" + phoneNumber + "|" + linkedBankUserId +
               "|" + linkedBankBankId + "|" + linkedBankAccountNumber;
    }

    User User::deserialize(const string &line) {
        stringstream ss(line);
        vector<string> parts;
        string part;

        while (getline(ss, part, '|')) {
            parts.push_back(part);
        }

        string id = parts.size() > 0 ? parts[0] : "";
        string name = parts.size() > 1 ? parts[1] : "";
        string username = parts.size() > 2 ? parts[2] : "";
        string password = parts.size() > 3 ? parts[3] : "";
        string roleStr = parts.size() > 4 ? parts[4] : "0";
        string cashStr = parts.size() > 5 ? parts[5] : "0";
        string phone = parts.size() > 6 ? parts[6] : "";
        string linkedBankUserId = parts.size() > 7 ? parts[7] : "";
        string linkedBankBankId = parts.size() > 8 ? parts[8] : "";
        string linkedBankAccountNumber = parts.size() > 9 ? parts[9] : "";

        int roleValue = 0;
        try {
            roleValue = stoi(roleStr);
        } catch (...) {
            roleValue = 0;
        }

        long long cashBalance = 0;
        try {
            cashBalance = stoll(cashStr);
        } catch (...) {
            cashBalance = 0;
        }

        User user(id, name, username, password, static_cast<Role>(roleValue));
        user.setCashBalance(cashBalance);
        user.setPhoneNumber(phone);
        user.setLinkedBankUserId(linkedBankUserId);
        user.setLinkedBankAccount(linkedBankBankId, linkedBankAccountNumber);
        return user;
    }
}
