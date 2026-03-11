#ifndef SECURITY_SCREENINGCLI_H
#define SECURITY_SCREENINGCLI_H

#include <string>

#include "../baggage/BaggageManager.h"
#include "../tickets/TicketManager.h"
#include "../users/UserManager.h"

namespace security {
    class ScreeningCLI {
    private:
        baggage::BaggageManager &baggageManager;
        tickets::TicketManager &ticketManager;
        users::UserManager &userManager;
    public:
        ScreeningCLI(baggage::BaggageManager &bm, tickets::TicketManager &tm, users::UserManager &um)
            : baggageManager(bm), ticketManager(tm), userManager(um) {}

        void runHandler(const std::string &currentUserId, Role currentRole);
    };
}

#endif
