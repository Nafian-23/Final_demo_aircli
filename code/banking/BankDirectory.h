#ifndef BANKING_BANKDIRECTORY_H
#define BANKING_BANKDIRECTORY_H

#include <string>
#include <vector>

#include "../Result.h"

namespace banking {

    struct Bank {
        std::string id;
        std::string name;
    };

    struct VerifiedBankAccount {
        std::string bankId;
        std::string bankName;
        std::string accountNumber;
        std::string customerId;
        std::string customerName;
        std::string phone;
    };

    class BankDirectory {
    public:
        static void ensureSeedBanks();
        static std::vector<Bank> listBanks();

        // Verifies that (bankId, accountNumber) exists and belongs to (phone, pin).
        // Returns Ok + VerifiedBankAccount on success; Err with message otherwise.
        static Result verifyAccountCredentials(
            const std::string &bankId,
            const std::string &accountNumber,
            const std::string &phone,
            const std::string &pin,
            VerifiedBankAccount &outVerified
        );

        // Debits money from a verified account if sufficient balance exists.
        // Uses the same bank storage files as BankCLI:
        // bank_accounts.txt and bank_tx.txt.
        static Result debitVerifiedAccount(
            const std::string &bankId,
            const std::string &accountNumber,
            const std::string &phone,
            const std::string &pin,
            long long amountCents,
            const std::string &description
        );

        // Practical booking payment:
        // Verify by account number + PIN (no separate terminal / no prior linking required).
        // Returns Ok on success, Err on mismatch or insufficient balance.
        static Result debitByAccountAndPin(
            const std::string &accountNumber,
            const std::string &pin,
            long long amountCents,
            const std::string &description
        );
    };
}

#endif
