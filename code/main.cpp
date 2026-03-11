#include <iostream>
#include "users/UserManager.h"
#include "users/UserCLI.h"
#include "banking/BankingManager.h"
#include "banking/BankingCLI.h"
#include "flights/FlightManager.h"
#include "flights/FlightsCLI.h"
#include "tickets/TicketManager.h"
#include "tickets/TicketsCLI.h"
#include "baggage/BaggageManager.h"
#include "baggage/BaggageCLI.h"
#include "security/ScreeningCLI.h"
#include "security/SeniorSecurityCLI.h"

int main() {
    // Initialize all our core services
    users::UserManager userManager;
    banking::BankingManager bankingManager;
    flights::FlightManager flightManager;
    tickets::TicketManager ticketManager;
    baggage::BaggageManager baggageManager;

    // Load existing data from storage
    userManager.load();
    bankingManager.load();
    flightManager.load();
    ticketManager.load();
    baggageManager.load();

    // make baggage class aware of manager for helper methods
    baggage::Baggage::setManager(&baggageManager);

    // Set up the CLI interfaces
    users::UserCLI userCLI(userManager);
    (void)bankingManager;
    banking::BankingCLI bankingCLI;
    flights::FlightsCLI flightsCLI(flightManager, ticketManager, userManager);
    tickets::TicketsCLI ticketsCLI(ticketManager, flightManager);
    baggage::BaggageCLI baggageCLI(baggageManager, ticketManager);
    security::ScreeningCLI screeningCLI(baggageManager, ticketManager, userManager);
    security::SeniorSecurityCLI seniorCLI(baggageManager);

    // Handle user authentication
    string userId;
    Role userRole = Role::Passenger;

    if (!userCLI.welcomeFlow(userId, userRole)) {
        return 0;
    }

    // Main dashboard loop
    bool isRunning = true;
    while (isRunning) {
        std::cout << "\n==============================\n";
        std::cout << " airCLI Dashboard\n";
        std::cout << "==============================\n";
        std::cout << "1. Search & Book Flights\n";
        std::cout << "2. My Tickets\n";
        std::cout << "3. Travel History\n";
        std::cout << "4. My Baggage\n";
        std::cout << "5. Link Bank\n";

        if (userRole == Role::BaggageHandler) {
            std::cout << "6. Baggage Screening (Handler)\n";
            std::cout << "7. Logout\n";
        } else if (userRole == Role::SeniorSecurity) {
            std::cout << "6. Senior Security Review\n";
            std::cout << "7. Logout\n";
        } else if (userRole == Role::Admin) {
            std::cout << "6. Admin: Manage Flights\n";
            std::cout << "7. Logout\n";
        } else {
            std::cout << "6. Logout\n";
        }
        
        std::cout << "\nSelect an option > ";

        int choice = 0;
        std::cin >> choice;

        switch (choice) {
        case 1:
            flightsCLI.runSearchAndBook(userId, userRole);
            break;
        case 2:
            ticketsCLI.runViewTickets(userId, userRole);
            break;
        case 3:
            ticketsCLI.runTravelHistory(userId, userRole);
            break;
        case 4: {
            baggageCLI.runPassenger(userId);
            break;
        }
        case 5: {
            bool openBanking = userCLI.linkBankAccount(userId);
            if (openBanking) bankingCLI.runBanking(userId, userRole);
            break;
        }
        case 6:
            if (userRole == Role::Admin) {
            flightsCLI.run(userId, userRole);
            } else if (userRole == Role::BaggageHandler) {
            screeningCLI.runHandler(userId, userRole);
            } else if (userRole == Role::SeniorSecurity) {
            seniorCLI.runSenior(userId, userRole);
            } else {
            // passenger or any other role without extra menu -> logout
            std::cout << "\nLogging out...\n";
            isRunning = false;
            }
            break;
        case 7:
            if (userRole == Role::Admin || userRole == Role::BaggageHandler || userRole == Role::SeniorSecurity) {
            std::cout << "\nLogging out...\n";
            isRunning = false;
            } else {
            std::cout << "\nInvalid option. Please try again.\n";
            }
            break;
        default:
            std::cout << "\nInvalid option. Please try again.\n";
        }
    }

    std::cout << "\nThanks for using airCLI. Goodbye!\n";
    return 0;
}
