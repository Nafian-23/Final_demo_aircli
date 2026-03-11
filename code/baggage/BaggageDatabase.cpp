#include "BaggageDatabase.h"

#include "../storage/DataStorage.h"

namespace baggage {

    bool BaggageDatabase::loadAll(std::map<std::string, Baggage> &outBaggage,
                                   std::vector<BaggageItem> &outItems) {
        outBaggage.clear();
        outItems.clear();

        auto bLines = storage::DataStorage::readAll("baggage_data.txt");
        for (const auto &l : bLines) {
            if (l.empty()) continue;
            Baggage b = Baggage::deserialize(l);
            if (!b.getId().empty()) outBaggage[b.getId()] = b;
        }

        auto iLines = storage::DataStorage::readAll("baggage_items.txt");
        for (const auto &l : iLines) {
            if (l.empty()) continue;
            outItems.push_back(BaggageItem::deserialize(l));
        }
        return true;
    }

    bool BaggageDatabase::saveAll(const std::map<std::string, Baggage> &inBaggage,
                                   const std::vector<BaggageItem> &inItems) {
        std::vector<std::string> bLines;
        bLines.reserve(inBaggage.size());
        for (const auto &p : inBaggage) bLines.push_back(p.second.serialize());
        if (!storage::DataStorage::writeAll("baggage_data.txt", bLines)) return false;

        std::vector<std::string> iLines;
        iLines.reserve(inItems.size());
        for (const auto &it : inItems) iLines.push_back(it.serialize());
        if (!storage::DataStorage::writeAll("baggage_items.txt", iLines)) return false;

        return true;
    }

    bool BaggageDatabase::appendLog(const std::string &filename, const std::string &line) {
        return storage::DataStorage::appendLine(filename, line);
    }

} // namespace baggage
