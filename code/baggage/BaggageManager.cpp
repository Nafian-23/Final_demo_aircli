#include "BaggageManager.h"
#include "BaggageDatabase.h"

#include "../storage/DataStorage.h"
#include <chrono>

using namespace std;

namespace baggage {

    static string newId(const string &prefix) {
        using namespace std::chrono;
        auto ts = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
        return prefix + to_string(ts);
    }

    bool BaggageManager::load() {
        baggageById.clear();
        items.clear();

        // delegate to database wrapper
        return BaggageDatabase::loadAll(baggageById, items);
    }

    bool BaggageManager::save() const {
        // delegate to database wrapper
        return BaggageDatabase::saveAll(baggageById, items);
    }

    const Baggage* BaggageManager::get(const string &id) const {
        auto it = baggageById.find(id);
        if (it == baggageById.end()) return nullptr;
        return &it->second;
    }

    vector<Baggage> BaggageManager::getForPassenger(const string &passengerId) const {
        vector<Baggage> out;
        for (const auto &p : baggageById) {
            if (p.second.getPassengerId() == passengerId) out.push_back(p.second);
        }
        return out;
    }

    vector<Baggage> BaggageManager::getByStatus(BaggageStatus status) const {
        vector<Baggage> out;
        for (const auto &p : baggageById) {
            if (p.second.getStatus() == status) out.push_back(p.second);
        }
        return out;
    }

    vector<BaggageItem> BaggageManager::getItems(const string &baggageId, ItemLayer layer) const {
        vector<BaggageItem> out;
        for (const auto &it : items) {
            if (it.baggageId == baggageId && it.layer == layer) out.push_back(it);
        }
        return out;
    }

    string BaggageManager::createDraftForTicket(
        const string &passengerId,
        const string &ticketId,
        const string &flightId,
        const string &origin,
        const string &destination
    ) {
        string id = newId("BG");
        while (baggageById.find(id) != baggageById.end()) id = newId("BG") + "x";
        Baggage b(id, passengerId, ticketId, flightId, origin, destination);
        b.setStatus(BaggageStatus::Draft);
        baggageById[id] = b;
        save();
        return id;
    }

    Result BaggageManager::addItem(const string &baggageId, ItemLayer layer, const string &name, const string &category, double weightKg) {
        auto it = baggageById.find(baggageId);
        if (it == baggageById.end()) return Result::Err("Baggage not found.");
        if (it->second.getStatus() != BaggageStatus::Draft) return Result::Err("Baggage is not editable.");
        if (name.empty() || weightKg <= 0.0) return Result::Err("Invalid item.");

        items.push_back(BaggageItem{baggageId, layer, name, category, weightKg});

        double declared = 0.0;
        double actual = 0.0;
        for (const auto &i : items) {
            if (i.baggageId != baggageId) continue;
            if (i.layer == ItemLayer::Declared) declared += i.weightKg;
            actual += i.weightKg;
        }
        it->second.setDeclaredWeightKg(declared);
        it->second.setActualWeightKg(actual);
        save();
        return Result::Ok();
    }

    Result BaggageManager::submit(const string &baggageId) {
        auto it = baggageById.find(baggageId);
        if (it == baggageById.end()) return Result::Err("Baggage not found.");
        if (it->second.getStatus() != BaggageStatus::Draft) return Result::Err("Baggage is already submitted.");
        it->second.setStatus(BaggageStatus::Submitted);
        save();
        return Result::Ok("Submitted.");
    }

    Result BaggageManager::setScreeningResult(const string &baggageId, int suspicionScore, bool escalated, const string &screenedByUserId, const string &notes) {
        auto it = baggageById.find(baggageId);
        if (it == baggageById.end()) return Result::Err("Baggage not found.");
        if (it->second.getStatus() != BaggageStatus::Submitted) return Result::Err("Baggage is not ready for screening.");
        it->second.setSuspicionScore(suspicionScore);
        it->second.setScreenedByUserId(screenedByUserId);
        it->second.setNotes(notes);
        it->second.setStatus(escalated ? BaggageStatus::Escalated : BaggageStatus::Cleared);
        save();
        return Result::Ok();
    }

    Result BaggageManager::setFinalDecision(const string &baggageId, bool invalidated, const string &decisionByUserId, const string &notes) {
        auto it = baggageById.find(baggageId);
        if (it == baggageById.end()) return Result::Err("Baggage not found.");
        if (it->second.getStatus() != BaggageStatus::Escalated) return Result::Err("Baggage is not escalated.");
        it->second.setFinalDecisionByUserId(decisionByUserId);
        it->second.setNotes(notes);
        it->second.setStatus(invalidated ? BaggageStatus::Invalidated : BaggageStatus::Cleared);
        save();
        return Result::Ok();
    }
}

