#include "FlightsCLI.h"
#include "../access/AccessControl.h"
#include <iostream>
#include "../banking/BankDirectory.h"

namespace flights {
    void FlightsCLI::run(const string &currentUserId, Role currentRole) {
        manager.load();
        ticketManager.load();
        cout << "\n-- Flights --\n";
        if (access::AccessControl::anyOf(currentRole, {Role::Admin})) {
            cout << "1) List Flights\n2) Create Flight\n3) Book Seat\n4) Back\nChoice: ";
        } else {
            cout << "1) List Flights\n2) Book Seat\n3) Back\nChoice: ";
        }
        int ch = 0; cin >> ch;
        if (ch == 1) {
            auto all = manager.all();
            cout << "Flights (" << all.size() << "):\n";
            for (const auto &f : all) {
                cout << "- " << f.getId() << " | " << f.getOrigin() << "->" << f.getDestination()
                          << " | " << f.getDate() << " | " << (f.getCapacity()-f.getSeatsBooked()) << " seats left\n";
            }
        } else if (ch == 2) {
            if (access::AccessControl::anyOf(currentRole, {Role::Admin})) {
                string o,d,date; int cap;
                cout << "Origin: "; cin >> o;
                cout << "Destination: "; cin >> d;
                cout << "Date(YYYY-MM-DD): "; cin >> date;
                cout << "Capacity: "; cin >> cap;
                auto id = manager.createFlight(o,d,date,cap);
                cout << "Created flight ID: " << id << "\n";
            } else {
                if (currentUserId.empty()) { cout << "No user selected.\n"; return; }
                string fid;
                cout << "Flight ID to book: "; cin >> fid;
                const Flight* flight = manager.get(fid);
                if (!flight) { cout << "Flight not found.\n"; return; }

                cout << "\n--- Payment ---\n";
                cout << "Bank account no: ";
                string accountNo;
                cin >> accountNo;
                cout << "Enter PIN: ";
                string pin;
                cin >> pin;

                const long long priceCents = 45000LL * 100LL;
                Result pay = banking::BankDirectory::debitByAccountAndPin(
                    accountNo,
                    pin,
                    priceCents,
                    "Flight booking " + fid
                );
                if (!pay.ok) {
                    cout << "\n❌ Payment failed: " << pay.message << "\n";
                    return;
                }

                bool ok = manager.bookSeat(fid);
                if (ok) {
                    auto ticketId = ticketManager.issueTicket(currentUserId, *flight);
                    cout << "\n✅ Payment successful (bank debited).\n";
                    cout << "🎟 Ticket ID: " << ticketId << "\n";
                } else {
                    cout << "Booking failed.\n";
                }
            }
        } else if (ch == 3) {
            if (access::AccessControl::anyOf(currentRole, {Role::Admin})) {
                cout << "Passenger ID: "; string passId; cin >> passId;
                string fid;
                cout << "Flight ID to book: "; cin >> fid;
                const Flight* flight = manager.get(fid);
                if (!flight) { cout << "Flight not found.\n"; return; }
                bool ok = manager.bookSeat(fid);
                if (ok) {
                    auto ticketId = ticketManager.issueTicket(passId, *flight);
                    cout << "Booked for " << passId << "! Ticket ID: " << ticketId << "\n";
                } else {
                    cout << "Booking failed.\n";
                }
            }
        }
    }

    void FlightsCLI::runSearchAndBook(const string &currentUserId, Role currentRole) {
        (void)currentRole;
        manager.load();
        ticketManager.load();
        userManager.load();
        
        cout << "\n======= Flight Search =======\n";
        cout << "1. Search by Route & Date\n";
        cout << "2. View All Flights\n";
        cout << "0. Back\n";
        cout << "\nSelect an option > ";
        
        int searchChoice = 0;
        cin >> searchChoice;
        
        if (searchChoice == 0) return;
        
        string from, to, date;
        if (searchChoice == 1) {
            cout << "\nFrom (Airport Code): ";
            cin >> from;
            cout << "To   (Airport Code): ";
            cin >> to;
            cout << "Date (YYYY-MM-DD)  : ";
            cin >> date;
        }
        
        auto allFlights = manager.all();
        vector<Flight> results;
        
        if (searchChoice == 1) {
            for (const auto &f : allFlights) {
                if (f.getOrigin() == from && f.getDestination() == to && f.getDate() == date) {
                    results.push_back(f);
                }
            }
        } else {
            results = allFlights;
        }
        
        if (results.empty()) {
            cout << "\nNo flights found matching your criteria.\n";
            return;
        }
        
        cout << "\nAvailable Flights:\n\n";
        for (size_t i = 0; i < results.size(); ++i) {
            const auto &f = results[i];
            int seatsAvailable = f.getCapacity() - f.getSeatsBooked();
            cout << "[" << (i+1) << "] " << f.getId() << " | " 
                      << f.getOrigin() << " → " << f.getDestination() << " | "
                      << f.getDate() << " | BDT 45,000 | " 
                      << seatsAvailable << " seats\n";
        }
        
        cout << "\n0. Cancel\nSelect a flight > ";
        int selection = 0;
        cin >> selection;
        
        if (selection < 1 || selection > (int)results.size()) {
            cout << "\nBooking cancelled.\n";
            return;
        }
        
        const Flight &selectedFlight = results[selection - 1];
        
        cout << "\nConfirm booking for " << selectedFlight.getId() << "\n";
        cout << "Price: BDT 45,000\n\n";
        cout << "Proceed? (y/n) > ";
        char proceed;
        cin >> proceed;
        
        if (proceed != 'y' && proceed != 'Y') {
            cout << "\nBooking cancelled.\n";
            return;
        }

        cout << "\n--- Payment ---\n";
        cout << "Bank account no: ";
        string accountNo;
        cin >> accountNo;
        cout << "Enter PIN: ";
        string pin;
        cin >> pin;

        const long long priceCents = 45000LL * 100LL;
        Result pr = banking::BankDirectory::debitByAccountAndPin(
            accountNo,
            pin,
            priceCents,
            "Flight booking " + selectedFlight.getId()
        );
        if (!pr.ok) {
            cout << "\n❌ Payment failed: " << pr.message << "\n";
            return;
        }
        
        bool booked = manager.bookSeat(selectedFlight.getId());
        if (!booked) {
            cout << "\n❌ Booking failed. No seats available.\n";
            return;
        }
        
        auto ticketId = ticketManager.issueTicket(currentUserId, selectedFlight);
        cout << "\n✅ Payment successful (bank debited).\n";
        cout << "🎟 Ticket ID: " << ticketId << "\n";
    }
}
