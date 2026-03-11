#ifndef BAGGAGE_SCREENINGMODULE_H
#define BAGGAGE_SCREENINGMODULE_H

#include <string>
#include "ScreeningReport.h"
#include "Baggage.h"   // need baggage definition

namespace tickets { class TicketManager; }
namespace users { class UserManager; }

namespace baggage {
    class ScreeningModule {
    public:
        // evaluate the given baggage and return a report; does not modify storage
        static ScreeningReport evaluate(const Baggage &bag,
                                        const tickets::TicketManager &ticketManager,
                                        const users::UserManager &userManager);

        // log a report to the persistent log (for auditing)
        static void logReport(const ScreeningReport &report, const std::string &handlerUserId);
    };
}

#endif
