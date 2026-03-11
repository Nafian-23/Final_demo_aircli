#ifndef SECURITY_SENIORSECURITYCLI_H
#define SECURITY_SENIORSECURITYCLI_H

#include <string>

#include "../baggage/BaggageManager.h"
#include "../Role.h"

namespace security {
    class SeniorSecurityCLI {
    private:
        baggage::BaggageManager &baggageManager;
    public:
        explicit SeniorSecurityCLI(baggage::BaggageManager &bm) : baggageManager(bm) {}

        void runSenior(const std::string &currentUserId, Role currentRole);
    };
}

#endif
