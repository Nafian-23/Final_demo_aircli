#include "BaggageCLI.h"

#include <iostream>
#include <limits>

using namespace std;

namespace baggage {

    static void clearLine() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    void BaggageCLI::runPassenger(const string &currentUserId) {
        manager.load();
        ticketManager.load();

        cout << "\n==============================\n";
        cout << " My Baggage\n";
        cout << "==============================\n";

        auto tickets = ticketManager.getForPassenger(currentUserId);
        if (tickets.empty()) {
            cout << "\nYou have no tickets yet. Book a flight first.\n";
            return;
        }

        cout << "\nSelect a ticket to create baggage for:\n";
        for (size_t i = 0; i < tickets.size(); i++) {
            cout << "  " << (i + 1) << ") " << tickets[i].getOrigin() << " -> " << tickets[i].getDestination()
                 << " | Flight " << tickets[i].getFlightId() << " | Ticket " << tickets[i].getId() << "\n";
        }
        cout << "Choice: ";
        int choice = 0;
        cin >> choice;
        clearLine();
        if (choice < 1 || choice > (int)tickets.size()) {
            cout << "\nInvalid choice.\n";
            return;
        }

        const auto &t = tickets[(size_t)choice - 1];
        const string baggageId = manager.createDraftForTicket(
            currentUserId, t.getId(), t.getFlightId(), t.getOrigin(), t.getDestination()
        );

        cout << "\nCreated baggage: " << baggageId << "\n";
        cout << "You can now add items.\n";

        auto addItems = [&](ItemLayer layer, const string &label) {
            while (true) {
                cout << "\nAdd " << label << " item? (y/n): ";
                char yn = 'n';
                cin >> yn;
                clearLine();
                if (!(yn == 'y' || yn == 'Y')) break;

                cout << "Item name: ";
                string name;
                getline(cin, name);
                cout << "Item category: ";
                string cat;
                getline(cin, cat);
                cout << "Item weight (kg): ";
                double w = 0.0;
                cin >> w;
                clearLine();

                Result r = manager.addItem(baggageId, layer, name, cat, w);
                if (!r.ok) {
                    cout << "\n✗ " << r.message << "\n";
                } else {
                    cout << "✓ Added.\n";
                }
            }
        };

        cout << "\n--- Declared Items (visible to screeners) ---\n";
        addItems(ItemLayer::Declared, "declared");

        cout << "\n--- Hidden Items (only Senior Security can access) ---\n";
        addItems(ItemLayer::Hidden, "hidden");

        manager.load();
        const Baggage *b = manager.get(baggageId);
        if (b) {
            cout << "\nDeclared weight: " << b->getDeclaredWeightKg() << " kg\n";
            cout << "Total (actual) weight: " << b->getActualWeightKg() << " kg\n";
        }

        cout << "\nSubmit baggage for screening? (y/n): ";
        char submit = 'n';
        cin >> submit;
        clearLine();
        if (submit == 'y' || submit == 'Y') {
            Result r = manager.submit(baggageId);
            if (r.ok) cout << "\n✓ Baggage submitted.\n";
            else cout << "\n✗ " << r.message << "\n";
        } else {
            cout << "\nBaggage left as Draft.\n";
        }
    }
}

