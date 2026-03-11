#include "AccessControl.h"

#include "../Result.h"
#include "../Role.h"

using namespace std;

namespace access {
    Result AccessControl::requireRole(Role currentRole, Role requiredRole) {
        if (currentRole == requiredRole) {
            return Result::Ok();
        }
        return Result::Err("Access denied: requires " + roleToString(requiredRole) + " role");
    }

    bool AccessControl::anyOf(Role currentRole, const std::initializer_list<Role> &allowedRoles) {
        for (auto role : allowedRoles) {
            if (currentRole == role) {
                return true;
            }
        }
        return false;
    }

    string AccessControl::roleToString(Role role) {
        switch (role) {
            case Role::Passenger:
                return "Passenger";
            case Role::Security:
                return "Security";
            case Role::SeniorSecurity:
                return "Senior Security";
            case Role::Admin:
                return "Administrator";
            case Role::BaggageHandler:
                return "Baggage Handler";
            default:
                return "Unknown";
        }
    }
}
