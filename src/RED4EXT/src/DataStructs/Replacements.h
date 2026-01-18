#pragma once
#include <memory>
#include <vector>

#include "RED4ext/CName.hpp"

namespace InfiniteRandomizerFramework
{
struct Replacements
{
    // weights[0] is reserved for the sum of all individual weights
    // weights at each given index besides 0 contain the sum of all previous weights
    std::unique_ptr<std::vector<float>> weights;
    std::unique_ptr<std::vector<RED4ext::CName>> appNames;
    std::unique_ptr<std::vector<RED4ext::ResourcePath>> resourcePaths;
};
}
