#pragma once

#include <vector>

#include "RED4ext/CName.hpp"
#include "RED4ext/ResourcePath.hpp"

namespace InfiniteRandomizerFramework {

    struct CategoryEntry {
        RED4ext::ResourcePath resourcePath;
        RED4ext::CName appearance;
    };

    struct Category {
        std::string extension;
        std::vector<CategoryEntry> entries;
    };
}
