#include "UserManager.h"
#include "../storage/DataStorage.h"
#include <chrono>

using namespace std;

namespace users {
    bool UserManager::load() {
        auto lines = storage::DataStorage::readAll("users.txt");
        usersById.clear();
        usernameToId.clear();
        for (const auto &l : lines) {
            User u = User::deserialize(l);
            usersById[u.getId()] = u;
            usernameToId[u.getUsername()] = u.getId();
        }
        return true;
    }

    bool UserManager::save() const {
        std::vector<std::string> lines;
        for (const auto &p : usersById) {
            lines.push_back(p.second.serialize());
        }
        return storage::DataStorage::writeAll("users.txt", lines);
    }

    string UserManager::createUser(const std::string &name, const std::string &username, const std::string &password, const std::string &phoneNumber) {
        using namespace std::chrono;
        auto t = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        string id = "U" + std::to_string(t);
        while (usersById.find(id) != usersById.end()) {
            ++t; id = "U" + std::to_string(t);
        }
        User u(id, name, username, password, Role::Passenger);
        u.setPhoneNumber(phoneNumber);
        usersById[id] = u;
        usernameToId[username] = id;
        storage::DataStorage::appendLine("users.txt", u.serialize());
        return id;
    }

    const User* UserManager::get(const string &id) const {
        auto it = usersById.find(id);
        if (it == usersById.end()) return nullptr;
        return &it->second;
    }

    const User* UserManager::findByUsername(const std::string &username) const {
        auto it = usernameToId.find(username);
        if (it == usernameToId.end()) return nullptr;
        return get(it->second);
    }

    const User* UserManager::findByPhoneNumber(const std::string &phoneNumber) const {
        for (const auto &p : usersById) {
            if (p.second.getPhoneNumber() == phoneNumber) {
                return &p.second;
            }
        }
        return nullptr;
    }

    bool UserManager::authenticate(const std::string &username, const std::string &password, string &outId) const {
        const User* user = findByUsername(username);
        if (!user) return false;
        if (user->getPassword() != password) return false;
        outId = user->getId();
        return true;
    }

    std::vector<User> UserManager::all() const {
        std::vector<User> v;
        v.reserve(usersById.size());
        for (const auto &p : usersById) v.push_back(p.second);
        return v;
    }

    bool UserManager::updateRole(const string &id, Role newRole) {
        auto it = usersById.find(id);
        if (it == usersById.end()) return false;
        it->second.setRole(newRole);
        return save();
    }

    bool UserManager::updateCashBalance(const string &id, long long amount) {
        auto it = usersById.find(id);
        if (it == usersById.end()) return false;
        it->second.addCash(amount);
        return save();
    }

    bool UserManager::updateCashBalanceByPhone(const string &phoneNumber, long long amount) {
        for (auto &p : usersById) {
            if (p.second.getPhoneNumber() == phoneNumber) {
                p.second.addCash(amount);
                return save();
            }
        }
        return false;
    }

    bool UserManager::updatePhoneNumber(const string &id, const string &phoneNumber) {
        auto it = usersById.find(id);
        if (it == usersById.end()) return false;
        it->second.setPhoneNumber(phoneNumber);
        return save();
    }

    bool UserManager::updateLinkedBankUserId(const string &id, const string &bankUserId) {
        auto it = usersById.find(id);
        if (it == usersById.end()) return false;
        it->second.setLinkedBankUserId(bankUserId);
        return save();
    }

    bool UserManager::updateLinkedBankAccount(const string &id, const string &bankId, const string &accountNumber) {
        auto it = usersById.find(id);
        if (it == usersById.end()) return false;
        it->second.setLinkedBankAccount(bankId, accountNumber);
        return save();
    }

    bool UserManager::clearLinkedBankAccount(const string &id) {
        auto it = usersById.find(id);
        if (it == usersById.end()) return false;
        it->second.clearLinkedBankAccount();
        return save();
    }

    bool UserManager::creditCashForLinkedBankAccount(const string &bankId, const string &accountNumber, long long amountCents) {
        for (auto &p : usersById) {
            if (p.second.getLinkedBankBankId() == bankId &&
                p.second.getLinkedBankAccountNumber() == accountNumber) {
                p.second.addCash(amountCents);
                return save();
            }
        }
        return false;
    }
}
