.PHONY: all aircli bankcli clean run run-bank data-dir

CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

ROOT_DIR := $(abspath .)
CODE_DIR := $(ROOT_DIR)/code
BUILD_DIR := $(ROOT_DIR)/build

INCLUDES := -I$(CODE_DIR)

# Main app (aircli) sources: everything needed except bankingMain.cpp
AIRCLI_SRCS := \
	$(CODE_DIR)/main.cpp \
	$(CODE_DIR)/access/AccessControl.cpp \
	$(CODE_DIR)/banking/BankingAccount.cpp \
	$(CODE_DIR)/banking/BankingCLI.cpp \
	$(CODE_DIR)/banking/BankingManager.cpp \
	$(CODE_DIR)/banking/BankDirectory.cpp \
	$(CODE_DIR)/banking/Transaction.cpp \
	$(CODE_DIR)/baggage/Baggage.cpp \
	$(CODE_DIR)/baggage/BaggageItem.cpp \
	$(CODE_DIR)/baggage/BaggageManager.cpp \
	$(CODE_DIR)/baggage/BaggageCLI.cpp \
	$(CODE_DIR)/baggage/BaggageDatabase.cpp \
	$(CODE_DIR)/baggage/ScreeningModule.cpp \
	$(CODE_DIR)/baggage/SeniorSecurityModule.cpp \
	$(CODE_DIR)/security/ScreeningCLI.cpp \
	$(CODE_DIR)/security/SeniorSecurityCLI.cpp \
	$(CODE_DIR)/flights/Flight.cpp \
	$(CODE_DIR)/flights/FlightManager.cpp \
	$(CODE_DIR)/flights/FlightsCLI.cpp \
	$(CODE_DIR)/logging/Logger.cpp \
	$(CODE_DIR)/storage/DataStorage.cpp \
	$(CODE_DIR)/tickets/Ticket.cpp \
	$(CODE_DIR)/tickets/TicketManager.cpp \
	$(CODE_DIR)/tickets/TicketsCLI.cpp \
	$(CODE_DIR)/users/User.cpp \
	$(CODE_DIR)/users/UserCLI.cpp \
	$(CODE_DIR)/users/UserManager.cpp

# Banking CLI sources (bankcli)
BANKCLI_SRCS := \
	$(CODE_DIR)/bankingMain.cpp \
	$(CODE_DIR)/access/AccessControl.cpp \
	$(CODE_DIR)/banking/BankingAccount.cpp \
	$(CODE_DIR)/banking/BankingManager.cpp \
	$(CODE_DIR)/banking/BankDirectory.cpp \
	$(CODE_DIR)/banking/Transaction.cpp \
	$(CODE_DIR)/logging/Logger.cpp \
	$(CODE_DIR)/storage/DataStorage.cpp \
	$(CODE_DIR)/users/User.cpp \
	$(CODE_DIR)/users/UserCLI.cpp \
	$(CODE_DIR)/users/UserManager.cpp

all: aircli bankcli

data-dir:
	@mkdir -p "$(ROOT_DIR)/storage/data"

aircli: data-dir
	@mkdir -p "$(BUILD_DIR)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(AIRCLI_SRCS) -o "$(BUILD_DIR)/aircli"

bankcli: data-dir
	@mkdir -p "$(BUILD_DIR)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(BANKCLI_SRCS) -o "$(BUILD_DIR)/bankcli"

run: aircli
	@"$(BUILD_DIR)/aircli"

run-bank: bankcli
	@"$(BUILD_DIR)/bankcli"

clean:
	@rm -rf "$(BUILD_DIR)"
