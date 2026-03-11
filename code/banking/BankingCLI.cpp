#include "BankingCLI.h"
#include <iostream>
#include <cstdlib>

using namespace std;

namespace banking {

    void BankingCLI::runBanking(const string &, Role) {
        cout << "\n==============================\n";
        cout << "Opening Banking CLI...\n";
        cout << "Running Banking CLI now.\n";
        cout << "==============================\n\n";

        // For demo reliability: run in the same terminal.
        // (A separate-terminal approach is platform-specific and often fails on macOS lab setups.)
        const string bankingCmd = "./build/bankcli";

        cout << "Launching Banking CLI...\n";
        const int result = system(("\"" + bankingCmd + "\"").c_str());
        if (result != 0) {
            cout << "\nFailed to run Banking CLI.\n";
            cout << "Please run: make bankcli && ./build/bankcli\n";
        }
    }

}
