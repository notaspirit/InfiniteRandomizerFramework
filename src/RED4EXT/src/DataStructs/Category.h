#pragma once

#include <vector>
#include "RED4ext/ResourcePath.hpp"

namespace InfiniteRandomizerFramework {

    struct CategoryEntry {
        const RED4ext::ResourcePath resourcePath;
        const char* appearance;
    };

    struct Category {
        const char* name;
        const char* extension;
        std::vector<CategoryEntry> entries;
    };
}
