#ifndef BAGGAGE_BAGGAGEMANAGER_H
#define BAGGAGE_BAGGAGEMANAGER_H

#include <map>
#include <vector>

#include "Baggage.h"
#include "BaggageItem.h"
#include "../Result.h"

namespace baggage {

    class BaggageManager {
    private:
        std::map<std::string, Baggage> baggageById;
        std::vector<BaggageItem> items;
    public:
        bool load();
        bool save() const;

        const Baggage* get(const std::string &id) const;
        std::vector<Baggage> getForPassenger(const std::string &passengerId) const;
        std::vector<Baggage> getByStatus(BaggageStatus status) const;

        std::vector<BaggageItem> getItems(const std::string &baggageId, ItemLayer layer) const;

        std::string createDraftForTicket(const std::string &passengerId,
                                         const std::string &ticketId,
                                         const std::string &flightId,
                                         const std::string &origin,
                                         const std::string &destination);

        Result addItem(const std::string &baggageId, ItemLayer layer, const std::string &name, const std::string &category, double weightKg);
        Result submit(const std::string &baggageId);

        Result setScreeningResult(const std::string &baggageId, int suspicionScore, bool escalated, const std::string &screenedByUserId, const std::string &notes);
        Result setFinalDecision(const std::string &baggageId, bool invalidated, const std::string &decisionByUserId, const std::string &notes);
    };
}

#endif
