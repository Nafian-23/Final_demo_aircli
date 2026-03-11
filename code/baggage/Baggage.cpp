#include "Baggage.h"
#include "BaggageManager.h"

#include <sstream>
#include <vector>

using namespace std;

namespace baggage {
    string Baggage::serialize() const {
        // id|passengerId|ticketId|flightId|origin|destination|declaredWeightKg|actualWeightKg|status|suspicionScore|screenedBy|decisionBy|notes
        ostringstream ds, as;
        ds.setf(std::ios::fixed); ds.precision(2); ds << declaredWeightKg;
        as.setf(std::ios::fixed); as.precision(2); as << actualWeightKg;

        return id + "|" + passengerId + "|" + ticketId + "|" + flightId + "|" + origin + "|" + destination +
               "|" + ds.str() + "|" + as.str() + "|" + to_string((int)status) + "|" + to_string(suspicionScore) +
               "|" + screenedByUserId + "|" + finalDecisionByUserId + "|" + notes;
    }

    static vector<string> splitPipe(const string &line) {
        vector<string> parts;
        string part;
        stringstream ss(line);
        while (getline(ss, part, '|')) parts.push_back(part);
        return parts;
    }

    Baggage Baggage::deserialize(const string &line) {
        auto p = splitPipe(line);
        Baggage b;
        b.id = p.size() > 0 ? p[0] : "";
        b.passengerId = p.size() > 1 ? p[1] : "";
        b.ticketId = p.size() > 2 ? p[2] : "";
        b.flightId = p.size() > 3 ? p[3] : "";
        b.origin = p.size() > 4 ? p[4] : "";
        b.destination = p.size() > 5 ? p[5] : "";
        try { b.declaredWeightKg = p.size() > 6 ? stod(p[6]) : 0.0; } catch (...) { b.declaredWeightKg = 0.0; }
        try { b.actualWeightKg = p.size() > 7 ? stod(p[7]) : 0.0; } catch (...) { b.actualWeightKg = 0.0; }
        int st = 0;
        try { st = p.size() > 8 ? stoi(p[8]) : 0; } catch (...) { st = 0; }
        b.status = (BaggageStatus)st;
        try { b.suspicionScore = p.size() > 9 ? stoi(p[9]) : 0; } catch (...) { b.suspicionScore = 0; }
        b.screenedByUserId = p.size() > 10 ? p[10] : "";
        b.finalDecisionByUserId = p.size() > 11 ? p[11] : "";
        b.notes = p.size() > 12 ? p[12] : "";
        return b;
    }

    string Baggage::statusToString(BaggageStatus s) {
        switch (s) {
            case BaggageStatus::Draft: return "Draft";
            case BaggageStatus::Submitted: return "Submitted";
            case BaggageStatus::Escalated: return "Escalated";
            case BaggageStatus::Cleared: return "Cleared";
            case BaggageStatus::Invalidated: return "Invalidated";
            default: return "Unknown";
        }
    }

    // manager pointer initialization
    BaggageManager *Baggage::manager = nullptr;

    void Baggage::setManager(BaggageManager *m) {
        manager = m;
    }

    Result Baggage::addItem(ItemLayer layer, const std::string &name, const std::string &category, double weightKg) {
        if (!manager) return Result::Err("No baggage manager attached.");
        return manager->addItem(id, layer, name, category, weightKg);
    }

    std::vector<BaggageItem> Baggage::getVisibleItems() const {
        if (!manager) return {};
        return manager->getItems(id, ItemLayer::Declared);
    }

    std::vector<BaggageItem> Baggage::getHiddenItems() const {
        if (!manager) return {};
        return manager->getItems(id, ItemLayer::Hidden);
    }

    double Baggage::calculateWeight() const {
        double total = 0.0;
        auto vis = getVisibleItems();
        for (auto &it : vis) total += it.weightKg;
        auto hid = getHiddenItems();
        for (auto &it : hid) total += it.weightKg;
        return total;
    }

    double Baggage::calculateExpectedWeight() const {
        double total = 0.0;
        auto vis = getVisibleItems();
        for (auto &it : vis) total += it.weightKg;
        return total;
    }
}

