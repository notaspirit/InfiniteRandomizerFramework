#pragma once

#include <vector>
#include "RED4ext/ResourcePath.hpp"

namespace InfiniteRandomizerFramework {

    struct VariantPoolEntry {
        RED4ext::ResourcePath resourcePath;
        std::string appearance;
        double weight;
    };

    struct VariantPool {
        std::string extension;
        std::string category;
        std::vector<VariantPoolEntry> entries;
    };
}
