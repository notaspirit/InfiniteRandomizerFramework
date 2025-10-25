#pragma once

#include <vector>
#include "RED4ext/ResourcePath.hpp"

namespace InfiniteRandomizerFramework {

    struct VariantPoolEntry {
        const RED4ext::ResourcePath resourcePath;
        const char* appearance;
        const float weight;
    };

    struct VariantPool {
        const char* name;
        const char* extension;
        const char* category;
        std::vector<VariantPoolEntry> entries;
    };
}
