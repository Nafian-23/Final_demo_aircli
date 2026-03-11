#ifndef BAGGAGE_ITEMDATABASE_H
#define BAGGAGE_ITEMDATABASE_H

#include <map>
#include <string>
#include "ItemCategory.h"

namespace baggage {
    struct ItemDefinition {
        std::string name;
        ItemCategory category;
        double typicalWeightKg;
        std::string description;
    };

    class ItemDatabase {
    private:
        std::map<std::string, ItemDefinition> items;
    public:
        ItemDatabase();

        // Load from file
        bool load();

        // Save to file
        bool save() const;

        // Get item definition by name (case-insensitive)
        const ItemDefinition* getItem(const std::string &name) const;

        // Add or update an item (admin only)
        void addItem(const std::string &name, ItemCategory category, double weightKg, const std::string &description);

        // Get all item names
        std::vector<std::string> getAllItemNames() const;

        // Suggest category for a name
        ItemCategory suggestCategory(const std::string &name) const;
    };
}

#endif