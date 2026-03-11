#include "SeniorSecurityCLI.h"

#include <iostream>
#include <limits>
#include <algorithm>
#include <chrono>

#include "../baggage/SeniorSecurityModule.h"
#include "../baggage/ScreeningReport.h"
#include "../baggage/Decision.h"

#include "../access/AccessControl.h"
#include "../storage/DataStorage.h"
#include "../Role.h"

using namespace std;

namespace security {

    static void clearLine() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    static long long nowSec() {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

    static bool containsKeyword(const string &haystack, const string &needleLower) {
        string h = haystack;
        transform(h.begin(), h.end(), h.begin(), ::tolower);
        return h.find(needleLower) != string::npos;
    }

    static vector<string> loadIllegalKeywords() {
        auto lines = storage::DataStorage::readAll("illegal_items.txt");
        vector<string> out;
        for (auto l : lines) {
            if (l.empty()) continue;
            transform(l.begin(), l.end(), l.begin(), ::tolower);
            out.push_back(l);
        }
        return out;
    }

    static void appendIncident(const string &baggageId, const string &passengerId, const string &flightId,
                               const string &decision, int score, const string &notes, const string &byUserId) {
        // id|ts|baggageId|passengerId|flightId|stage|decision|score|notes|byUserId
        const string id = "SI" + to_string(nowSec());
        const string line =
            id + "|" + to_string(nowSec()) + "|" + baggageId + "|" + passengerId + "|" + flightId + "|" +
            "SENIOR_REVIEW" + "|" + decision + "|" + to_string(score) + "|" + notes + "|" + byUserId;
        storage::DataStorage::appendLine("incidents.txt", line);
    }

    void SeniorSecurityCLI::runSenior(const string &currentUserId, Role currentRole) {
        if (!access::AccessControl::anyOf(currentRole, {Role::SeniorSecurity})) {
            cout << "\nAccess denied: requires Senior Security role.\n";
            return;
        }

        baggageManager.load();
        auto escalated = baggageManager.getByStatus(baggage::BaggageStatus::Escalated);

        cout << "\n==============================\n";
        cout << " Senior Security Review\n";
        cout << "==============================\n";

        if (escalated.empty()) {
            cout << "\nNo escalated baggage waiting.\n";
            return;
        }

        cout << "\nEscalated baggage list:\n";
        for (size_t i = 0; i < escalated.size(); i++) {
            cout << "  " << (i + 1) << ") " << escalated[i].getId()
                 << " | Passenger " << escalated[i].getPassengerId()
                 << " | " << escalated[i].getOrigin() << " -> " << escalated[i].getDestination()
                 << " | Score " << escalated[i].getSuspicionScore() << "\n";
        }
        cout << "Choice: ";
        int choice = 0;
        cin >> choice;
        clearLine();
        if (choice < 1 || choice > (int)escalated.size()) {
            cout << "\nInvalid choice.\n";
            return;
        }

        const auto bag = escalated[(size_t)choice - 1];
        auto declared = baggageManager.getItems(bag.getId(), baggage::ItemLayer::Declared);
        auto hidden = baggageManager.getItems(bag.getId(), baggage::ItemLayer::Hidden);

        // reconstruct screening report so module can evaluate
        baggage::ScreeningReport report;
        report.baggageId = bag.getId();
        report.passengerId = bag.getPassengerId();
        report.flightId = bag.getFlightId();
        report.origin = bag.getOrigin();
        report.destination = bag.getDestination();
        report.expectedWeightKg = bag.getDeclaredWeightKg();
        report.actualWeightKg = bag.getActualWeightKg();
        report.visibleItems = declared;
        report.suspicionScore = bag.getSuspicionScore();
        report.escalated = true;

        cout << "\n--- Deep Inspection ---\n";
        cout << "Baggage: " << report.baggageId << "\n";
        cout << "Passenger: " << report.passengerId << "\n";
        cout << "Route: " << report.origin << " -> " << report.destination << "\n";
        cout << "Declared weight: " << report.expectedWeightKg << " kg\n";
        cout << "Actual weight  : " << report.actualWeightKg << " kg\n";

        cout << "\nDeclared items:\n";
        if (declared.empty()) cout << "  (none)\n";
        for (const auto &it : declared) cout << "  - " << it.name << " (" << it.category << ", " << it.weightKg << " kg)\n";

        cout << "\nHidden items (Senior Security only):\n";
        if (hidden.empty()) cout << "  (none)\n";
        for (const auto &it : hidden) cout << "  - " << it.name << " (" << it.category << ", " << it.weightKg << " kg)\n";

        const auto illegal = loadIllegalKeywords();
        string notes;
        baggage::Decision finalDecision = baggage::SeniorSecurityModule::review(report, hidden, illegal, notes);

        switch (finalDecision) {
            case baggage::Decision::Clear:
                cout << "\nDecision: CLEAR passenger\n";
                break;
            case baggage::Decision::Warn:
                cout << "\nDecision: WARN (rule violation)\n";
                break;
            case baggage::Decision::Invalidate:
                cout << "\nDecision: INVALIDATE passenger\n";
                break;
        }

        // require notes
        if (notes.empty()) notes = "Reviewed.";

        bool invalidated = (finalDecision == baggage::Decision::Invalidate);
        Result r = baggageManager.setFinalDecision(bag.getId(), invalidated, currentUserId, notes);
        if (!r.ok) {
            cout << "\n✗ " << r.message << "\n";
            return;
        }

        baggage::SeniorSecurityModule::logDecision(bag.getId(), finalDecision, currentUserId, notes);

        appendIncident(
            bag.getId(),
            bag.getPassengerId(),
            bag.getFlightId(),
            invalidated ? "INVALIDATE" : "CLEAR",
            bag.getSuspicionScore(),
            notes,
            currentUserId
        );

        cout << "\n✓ Decision saved.\n";
    }
}

