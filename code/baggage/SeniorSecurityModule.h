#ifndef BAGGAGE_SENIORSECURITYMODULE_H
#define BAGGAGE_SENIORSECURITYMODULE_H

#include <vector>
#include <string>
#include "ScreeningReport.h"
#include "Decision.h"
#include "BaggageItem.h"

namespace baggage {
    class SeniorSecurityModule {
    public:
        // perform a review of an escalated screening report; returns final decision
        // notes may be populated with details of the decision
        static Decision review(const ScreeningReport &report,
                               const std::vector<BaggageItem> &hiddenItems,
                               const std::vector<std::string> &illegalKeywords,
                               std::string &notes);

        // log a senior decision for persistence / audit
        static void logDecision(const std::string &baggageId,
                                Decision decision,
                                const std::string &byUserId,
                                const std::string &notes);
    };
}

#endif
