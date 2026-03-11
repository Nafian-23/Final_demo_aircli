#ifndef ROLE_H
#define ROLE_H

enum class Role {
        Passenger = 0,      // Regular passenger user
        Security = 1,       // Security officer
        SeniorSecurity = 2, // Senior security manager
        Admin = 3,          // System administrator
        BaggageHandler = 4  // Baggage handler (screening operator)
    };

#endif