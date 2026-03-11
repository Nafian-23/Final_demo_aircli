#ifndef BAGGAGE_SCREENINGREPORT_H
#define BAGGAGE_SCREENINGREPORT_H

#include <string>
#include <vector>

#include "BaggageItem.h"

namespace baggage {

struct ScreeningReport {
    std::string baggageId;
    std::string passengerId;
    std::string flightId;
    std::string origin;
    std::string destination;
    double expectedWeightKg = 0.0;
    double actualWeightKg = 0.0;
    std::vector<BaggageItem> visibleItems;
    int suspicionScore = 0;
    bool escalated = false;
    std::string notes;
};

} // namespace baggage

#endif
