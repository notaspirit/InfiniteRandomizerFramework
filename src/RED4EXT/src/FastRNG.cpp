#include "FastRNG.h"

namespace InfiniteRandomizerFramework {
    void FastRNG::xorshift32() {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
    }

    uint32_t FastRNG::getInt32(const uint32_t max, const uint32_t min) {
        xorshift32();
        return min + (state % (max - min));
    }

    float FastRNG::getFloat(const float max, const float min) {
        xorshift32();
        return min + (max - min) * state * (1.0f / 4294967296.0f);
    }
}