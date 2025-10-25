#pragma once
#include <memory>
#include <vector>

#include "RED4ext/CName.hpp"

namespace InfiniteRandomizerFramework
{
struct Replacements
{
    // weights[0] is reserved for the sum of all weights
    // if the sum of all weights is equal to the amount of entries,
    // then it is negative and the absolute value is equal to the amount of entries.
    std::unique_ptr<std::vector<float>> weights;
    std::unique_ptr<std::vector<RED4ext::CName>> appNames;
    std::unique_ptr<std::vector<uint64_t>> resourcePaths;
};
}
