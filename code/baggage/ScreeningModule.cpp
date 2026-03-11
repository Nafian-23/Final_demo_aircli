#include "ScreeningModule.h"

#include <cmath>
#include <algorithm>
#include <vector>
#include <chrono>

#include "../storage/DataStorage.h"
#include "BaggageManager.h"

// stand‑alone incident counter (similar logic used in ScreeningCLI)
static int priorIncidentCountForPassenger(const std::string &passengerId) {
    auto lines = storage::DataStorage::readAll("incidents.txt");
    int count = 0;
    for (const auto &l : lines) {
        if (l.empty()) continue;
        if (l.find("|" + passengerId + "|") != std::string::npos) count++;
    }
    return count;
}

namespace baggage {

    static long long nowSec() {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

    // similar helper from CLI but exposed here for modules
    static bool containsKeyword(const std::string &haystack, const std::string &needleLower) {
        std::string h = haystack;
        std::transform(h.begin(), h.end(), h.begin(), ::tolower);
        return h.find(needleLower) != std::string::npos;
    }

    // very basic restricted route mapping for demonstration
    static bool routeHasRestriction(const std::string &origin, const std::string &dest,
                                    const BaggageItem &item) {
        // for example, flights arriving in "LON" cannot carry liquids
        if (dest == "LON") {
            if (containsKeyword(item.name, "liquid") || containsKeyword(item.category, "liquid"))
                return true;
        }
        // add other sample rules here
        return false;
    }

    ScreeningReport ScreeningModule::evaluate(const Baggage &bag,
                                               const tickets::TicketManager &ticketManager,
                                               const users::UserManager &userManager) {
        ScreeningReport rep;
        rep.baggageId = bag.getId();
        rep.passengerId = bag.getPassengerId();
        rep.flightId = bag.getFlightId();
        rep.origin = bag.getOrigin();
        rep.destination = bag.getDestination();
        rep.expectedWeightKg = bag.calculateExpectedWeight();
        rep.actualWeightKg = bag.calculateWeight();
        rep.visibleItems = bag.getVisibleItems();
        rep.notes = "";

        int score = 0;
        // weight mismatch factor
        if (rep.expectedWeightKg > 0.0) {
            double diffPercent = (rep.actualWeightKg - rep.expectedWeightKg) / rep.expectedWeightKg * 100.0;
            if (diffPercent > 20.0) score += 35;
            else if (diffPercent > 10.0) score += 20;
            // else no penalty
        }

        // route-item inconsistency
        for (auto &it : rep.visibleItems) {
            if (routeHasRestriction(rep.origin, rep.destination, it)) {
                score += 45;
                break;
            }
        }

        // passenger history factor
        int incidents = priorIncidentCountForPassenger(rep.passengerId);
        if (incidents > 0) {
            score += std::min(incidents * 25, 30);
        }

        if (score < 0) score = 0;
        if (score > 100) score = 100;
        rep.suspicionScore = score;
        rep.escalated = (score >= 60);
        return rep;
    }

    void ScreeningModule::logReport(const ScreeningReport &report, const std::string &handlerUserId) {
        // log to a dedicated file for audits
        // format: id|ts|baggageId|passengerId|flightId|score|escalated|handlerId
        std::string id = "SC" + std::to_string(nowSec());
        std::string line = id + "|" + std::to_string(nowSec()) + "|" + report.baggageId + "|" +
                           report.passengerId + "|" + report.flightId + "|" +
                           std::to_string(report.suspicionScore) + "|" +
                           (report.escalated ? "1" : "0") + "|" + handlerUserId + "|" + report.notes;
        storage::DataStorage::appendLine("screening_log.txt", line);
    }

} // namespace baggage
