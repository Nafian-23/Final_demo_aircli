#ifndef BAGGAGE_BAGGAGECLI_H
#define BAGGAGE_BAGGAGECLI_H

#include <string>

#include "BaggageManager.h"
#include "../tickets/TicketManager.h"

namespace baggage {
    class BaggageCLI {
    private:
        BaggageManager &manager;
        tickets::TicketManager &ticketManager;
    public:
        BaggageCLI(BaggageManager &m, tickets::TicketManager &tm) : manager(m), ticketManager(tm) {}

        void runPassenger(const std::string &currentUserId);
    };
}

#endif
