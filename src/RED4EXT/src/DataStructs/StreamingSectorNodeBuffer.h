#pragma once

#include "RED4ext/Scripting/Natives/Generated/world/Node.hpp"
#include <RED4ext/Scripting/Natives/Generated/world/StreamingSector.hpp>

namespace InfiniteRandomizerFramework
{

    struct StreamingSectorNodeBuffer
    {
        uint8_t pad00[0x28];
        RED4ext::DynArray<RED4ext::Handle<RED4ext::worldNode>> nodes;
    };

    inline RED4ext::DynArray<RED4ext::Handle<RED4ext::worldNode>>& GetNodes(RED4ext::world::StreamingSector* sector)
    {
        auto* nodeBuffer = reinterpret_cast<StreamingSectorNodeBuffer*>(reinterpret_cast<uint8_t*>(sector) + 0x40);
        return nodeBuffer->nodes;
    }

};