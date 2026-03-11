#include "BaggageItem.h"

#include <sstream>
#include <vector>

using namespace std;

namespace baggage {
    string BaggageItem::serialize() const {
        // baggageId|layer|name|category|weightKg
        ostringstream ws;
        ws.setf(std::ios::fixed);
        ws.precision(2);
        ws << weightKg;
        return baggageId + "|" + to_string((int)layer) + "|" + name + "|" + category + "|" + ws.str();
    }

    static vector<string> splitPipe(const string &line) {
        vector<string> parts;
        string part;
        stringstream ss(line);
        while (getline(ss, part, '|')) parts.push_back(part);
        return parts;
    }

    BaggageItem BaggageItem::deserialize(const string &line) {
        auto p = splitPipe(line);
        BaggageItem it;
        it.baggageId = p.size() > 0 ? p[0] : "";
        int layerVal = 0;
        try { layerVal = p.size() > 1 ? stoi(p[1]) : 0; } catch (...) { layerVal = 0; }
        it.layer = (ItemLayer)layerVal;
        it.name = p.size() > 2 ? p[2] : "";
        it.category = p.size() > 3 ? p[3] : "";
        try { it.weightKg = p.size() > 4 ? stod(p[4]) : 0.0; } catch (...) { it.weightKg = 0.0; }
        return it;
    }
}

