#ifndef BANKING_BANKINGCLI_H
#define BANKING_BANKINGCLI_H

#include <string>
#include "../Role.h"

namespace banking {
    
    class BankingCLI {
    public:
        
        BankingCLI() = default;

        // Admin banking interface
        // void run(const utils::ID &userId, utils::Role userRole);

        // User banking interface
        void runBanking(const std::string &userId, Role userRole);
    };
}

#endif
