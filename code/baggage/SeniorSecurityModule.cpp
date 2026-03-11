#include "SeniorSecurityModule.h"

#include <chrono>
#include <sstream>
#include <algorithm>

#include "../storage/DataStorage.h"

namespace baggage {

    static long long nowSec() {
        using namespace std::chrono;
        return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
    }

    static bool containsKeyword(const std::string &haystack, const std::string &needleLower) {
        std::string h = haystack;
        std::transform(h.begin(), h.end(), h.begin(), ::tolower);
        return h.find(needleLower) != std::string::npos;
    }

    Decision SeniorSecurityModule::review(const ScreeningReport &report,
                                           const std::vector<BaggageItem> &hiddenItems,
                                           const std::vector<std::string> &illegalKeywords,
                                           std::string &notes) {
        bool illegalFound = false;
        bool anyHidden = !hiddenItems.empty();

        // scan hidden and visible items for illegal keywords
        for (auto &it : hiddenItems) {
            for (auto &kw : illegalKeywords) {
                if (containsKeyword(it.name, kw) || containsKeyword(it.category, kw)) {
                    illegalFound = true;
                    notes = "Illegal item found: " + it.name;
                    return Decision::Invalidate;
                }
            }
            // check undeclared cash pattern e.g. "cash5000" or "cash 6000"
            if (containsKeyword(it.name, "cash")) {
                std::string digits;
                for (char c : it.name) if (isdigit(c)) digits.push_back(c);
                if (!digits.empty()) {
                    try {
                        int amount = stoi(digits);
                        if (amount > 5000) {
                            illegalFound = true;
                            notes = "Undeclared cash > $5000: " + digits;
                            return Decision::Invalidate;
                        }
                    } catch (...) {}
                }
            }
        }

        // if no illegal items but there were hidden items, issue warning
        if (!illegalFound && anyHidden) {
            notes = "Hidden items present but not illegal.";
            return Decision::Warn;
        }

        notes = "No issues detected.";
        return Decision::Clear;
    }

    void SeniorSecurityModule::logDecision(const std::string &baggageId,
                                           Decision decision,
                                           const std::string &byUserId,
                                           const std::string &notes) {
        // format: id|ts|baggageId|decision|byUser|notes
        std::string id = "SD" + std::to_string(nowSec());
        std::ostringstream ss;
        ss << id << "|" << nowSec() << "|" << baggageId << "|";
        switch (decision) {
            case Decision::Clear: ss << "CLEAR"; break;
            case Decision::Warn: ss << "WARN"; break;
            case Decision::Invalidate: ss << "INVALIDATE"; break;
        }
        ss << "|" << byUserId << "|" << notes;
        storage::DataStorage::appendLine("escalations.txt", ss.str());
    }

} // namespace baggage
