#pragma once

#include <vector>
#include "RED4ext/ResourcePath.hpp"

namespace InfiniteRandomizerFramework {

    struct CategoryEntry {
        RED4ext::ResourcePath resourcePath;
        std::string appearance;
    };

    struct Category {
        std::string extension;
        std::vector<CategoryEntry> entries;
    };
}
