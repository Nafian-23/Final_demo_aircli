#ifndef FLIGHTS_FLIGHTSCLI_H
#define FLIGHTS_FLIGHTSCLI_H

#include <string>
#include "FlightManager.h"
#include "../Role.h"
#include "../tickets/TicketManager.h"
#include "../users/UserManager.h"

namespace flights {
    class FlightsCLI {
    private:
        FlightManager &manager;
        tickets::TicketManager &ticketManager;
        users::UserManager &userManager;
    public:
        explicit FlightsCLI(FlightManager &m, tickets::TicketManager &tm, users::UserManager &um)
            : manager(m), ticketManager(tm), userManager(um) {}
        void run(const string &currentUserId, Role currentRole);
        void runSearchAndBook(const string &currentUserId, Role currentRole);
    };
}

#endif
