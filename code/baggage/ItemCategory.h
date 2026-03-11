#ifndef BAGGAGE_ITEMCATEGORY_H
#define BAGGAGE_ITEMCATEGORY_H

#include <string>

namespace baggage {
    enum class ItemCategory {
        Clothing,
        Electronics,
        Toiletries,
        Food,
        Documents,
        Liquids,
        SharpObjects,
        Cash,
        Other
    };

    // Convert enum to string
    std::string categoryToString(ItemCategory cat);

    // Convert string to enum (case-insensitive)
    ItemCategory stringToCategory(const std::string &str);

    // Get list of all categories as strings
    std::vector<std::string> getAllCategories();
}

#endif