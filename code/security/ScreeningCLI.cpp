#include "ScreeningCLI.h"

#include <iostream>
#include <limits>
#include <cmath>
#include <algorithm>
#include <chrono>

#include "../baggage/ScreeningModule.h"
#include "../baggage/ScreeningReport.h"

#include "../access/AccessControl.h"
#include "../storage/DataStorage.h"

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

    static int priorIncidentCountForPassenger(const string &passengerId) {
        auto lines = storage::DataStorage::readAll("incidents.txt");
        int count = 0;
        for (const auto &l : lines) {
            if (l.empty()) continue;
            // id|ts|baggageId|passengerId|flightId|stage|decision|score|notes|byUserId
            if (l.find("|" + passengerId + "|") != string::npos) count++;
        }
        return count;
    }

    static void appendIncident(const string &baggageId, const string &passengerId, const string &flightId,
                               const string &stage, const string &decision, int score, const string &notes, const string &byUserId) {
        const string id = "SI" + to_string(nowSec());
        const string line =
            id + "|" + to_string(nowSec()) + "|" + baggageId + "|" + passengerId + "|" + flightId + "|" +
            stage + "|" + decision + "|" + to_string(score) + "|" + notes + "|" + byUserId;
        storage::DataStorage::appendLine("incidents.txt", line);
    }

    void ScreeningCLI::runHandler(const string &currentUserId, Role currentRole) {
        if (!access::AccessControl::anyOf(currentRole, {Role::BaggageHandler})) {
            cout << "\nAccess denied: requires Baggage Handler role.\n";
            return;
        }

        baggageManager.load();
        ticketManager.load();
        userManager.load();

        auto pending = baggageManager.getByStatus(baggage::BaggageStatus::Submitted);
        cout << "\n==============================\n";
        cout << " Baggage Screening (Handler)\n";
        cout << "==============================\n";

        if (pending.empty()) {
            cout << "\nNo submitted baggage pending screening.\n";
            return;
        }

        cout << "\nSubmitted baggage list:\n";
        for (size_t i = 0; i < pending.size(); i++) {
            cout << "  " << (i + 1) << ") " << pending[i].getId()
                 << " | Passenger " << pending[i].getPassengerId()
                 << " | " << pending[i].getOrigin() << " -> " << pending[i].getDestination()
                 << " | Declared " << pending[i].getDeclaredWeightKg() << " kg\n";
        }
        cout << "Choice: ";
        int choice = 0;
        cin >> choice;
        clearLine();
        if (choice < 1 || choice > (int)pending.size()) {
            cout << "\nInvalid choice.\n";
            return;
        }

        // delegate screening logic to module
        const auto bag = pending[(size_t)choice - 1];
        baggage::ScreeningReport report = baggage::ScreeningModule::evaluate(bag, ticketManager, userManager);

        cout << "\n--- Non-invasive inspection ---\n";
        cout << "Baggage: " << report.baggageId << "\n";
        cout << "Route  : " << report.origin << " -> " << report.destination << "\n";
        cout << "Expected weight: " << report.expectedWeightKg << " kg\n";
        cout << "Actual weight  : " << report.actualWeightKg << " kg\n";
        cout << "\nDeclared items:\n";
        if (report.visibleItems.empty()) {
            cout << "  (none)\n";
        } else {
            for (const auto &it : report.visibleItems) {
                cout << "  - " << it.name << " (" << it.category << ", " << it.weightKg << " kg)\n";
            }
        }

        cout << "\n--- Screening Result ---\n";
        cout << "Suspicion score: " << report.suspicionScore << "/100\n";
        cout << "Decision: " << (report.escalated ? "ESCALATE" : "CLEAR") << "\n";

        cout << "Add notes (optional): ";
        string notes;
        getline(cin, notes);
        report.notes = notes;

        Result r = baggageManager.setScreeningResult(bag.getId(), report.suspicionScore, report.escalated, currentUserId, notes);
        if (!r.ok) {
            cout << "\n✗ " << r.message << "\n";
            return;
        }

        // log via module too
        baggage::ScreeningModule::logReport(report, currentUserId);

        appendIncident(
            bag.getId(),
            bag.getPassengerId(),
            bag.getFlightId(),
            "HANDLER_SCREEN",
            report.escalated ? "ESCALATE" : "CLEAR",
            report.suspicionScore,
            notes,
            currentUserId
        );

        cout << "\n✓ Screening saved.\n";
    }
}

