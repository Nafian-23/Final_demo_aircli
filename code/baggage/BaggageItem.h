#ifndef BAGGAGE_BAGGAGEITEM_H
#define BAGGAGE_BAGGAGEITEM_H

#include <string>

namespace baggage {
    enum class ItemLayer {
        Declared = 0,
        Hidden = 1
    };

    struct BaggageItem {
        std::string baggageId;
        ItemLayer layer = ItemLayer::Declared;
        std::string name;
        std::string category;          // added category
        double weightKg = 0.0;

        std::string serialize() const;
        static BaggageItem deserialize(const std::string &line);
    };
}

#endif
