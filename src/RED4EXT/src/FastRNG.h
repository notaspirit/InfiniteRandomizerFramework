#pragma once
#include <cstdint>

namespace InfiniteRandomizerFramework {
    struct FastRNG {
        uint32_t state;
        void xorshift32();
        uint32_t getInt32(uint32_t max, uint32_t min = 0);
        float getFloat(float max, float min = 0);
    };
}
