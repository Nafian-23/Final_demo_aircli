#ifndef BAGGAGE_BAGGAGEDATABASE_H
#define BAGGAGE_BAGGAGEDATABASE_H

#include <map>
#include <vector>
#include "Baggage.h"
#include "BaggageItem.h"

namespace baggage {
    class BaggageDatabase {
    public:
        // read/write baggage records
        static bool loadAll(std::map<std::string, Baggage> &outBaggage,
                            std::vector<BaggageItem> &outItems);
        static bool saveAll(const std::map<std::string, Baggage> &inBaggage,
                            const std::vector<BaggageItem> &inItems);

        // append a generic line to a log file (incidents/escalations etc)
        static bool appendLog(const std::string &filename, const std::string &line);
    };
}

#endif
