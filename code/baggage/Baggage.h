#ifndef BAGGAGE_BAGGAGE_H
#define BAGGAGE_BAGGAGE_H

#include <string>
#include <vector>
#include "../Result.h"
#include "BaggageItem.h"

namespace baggage {

    enum class BaggageStatus {
        Draft = 0,
        Submitted = 1,
        Escalated = 2,
        Cleared = 3,
        Invalidated = 4
    };

    class Baggage {
    private:
        std::string id;
        std::string passengerId;
        std::string ticketId;
        std::string flightId;
        std::string origin;
        std::string destination;
        double declaredWeightKg = 0.0;
        double actualWeightKg = 0.0;
        BaggageStatus status = BaggageStatus::Draft;
        int suspicionScore = 0;
        std::string screenedByUserId;
        std::string finalDecisionByUserId;
        std::string notes;
        // items are stored externally by manager; methods delegate there
    public:
        // allow modules to interact with a manager instance
        static class BaggageManager *manager;
        static void setManager(class BaggageManager *m);

        // convenience methods that mirror manager behavior
        struct ItemInfo { std::string name; std::string category; double weightKg; bool isHidden; };
        Result addItem(ItemLayer layer, const std::string &name, const std::string &category, double weightKg);
        std::vector<BaggageItem> getVisibleItems() const;
        std::vector<BaggageItem> getHiddenItems() const;
        double calculateWeight() const;
        double calculateExpectedWeight() const;
        Baggage() = default;
        Baggage(const std::string &id,
                const std::string &passengerId,
                const std::string &ticketId,
                const std::string &flightId,
                const std::string &origin,
                const std::string &destination)
            : id(id),
              passengerId(passengerId),
              ticketId(ticketId),
              flightId(flightId),
              origin(origin),
              destination(destination) {}

        const std::string &getId() const { return id; }
        const std::string &getPassengerId() const { return passengerId; }
        const std::string &getTicketId() const { return ticketId; }
        const std::string &getFlightId() const { return flightId; }
        const std::string &getOrigin() const { return origin; }
        const std::string &getDestination() const { return destination; }

        double getDeclaredWeightKg() const { return declaredWeightKg; }
        double getActualWeightKg() const { return actualWeightKg; }
        void setDeclaredWeightKg(double v) { declaredWeightKg = v; }
        void setActualWeightKg(double v) { actualWeightKg = v; }

        BaggageStatus getStatus() const { return status; }
        void setStatus(BaggageStatus s) { status = s; }

        int getSuspicionScore() const { return suspicionScore; }
        void setSuspicionScore(int s) { suspicionScore = s; }

        const std::string &getScreenedByUserId() const { return screenedByUserId; }
        void setScreenedByUserId(const std::string &u) { screenedByUserId = u; }

        const std::string &getFinalDecisionByUserId() const { return finalDecisionByUserId; }
        void setFinalDecisionByUserId(const std::string &u) { finalDecisionByUserId = u; }

        const std::string &getNotes() const { return notes; }
        void setNotes(const std::string &n) { notes = n; }

        std::string serialize() const;
        static Baggage deserialize(const std::string &line);

        static std::string statusToString(BaggageStatus s);
    };
}

#endif
