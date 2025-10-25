#include "InfiniteRandomizerFrameworkNative.h"

#include "DataStructs/StreamingSectorNodeBuffer.h"
#include "RED4ext/Scripting/Utils.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/StreamingSector.hpp"
#include "RedLib.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/MeshNode.hpp"

namespace InfiniteRandomizerFramework {
    void InfiniteRandomizerFrameworkNative::OnSectorPostLoad(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4) {
        RED4ext::Handle<RED4ext::worldStreamingSector> sector;
        RED4ext::GetParameter(aFrame, &sector);
        aFrame->code++;

        if (!m_initialized)
        {
            return;
        }

        for (auto& nodes = GetNodes(sector); const auto& node : nodes)
        {
            if (node->GetNativeType()->IsA(m_rttis->GetType("worldMeshNode")))
            {
                const auto meshNode = Red::Cast<RED4ext::worldMeshNode>(node);
                constexpr auto replacementPath = RED4ext::ResourcePath("cyberZ(h)ines\\meshes\\cyberzhines.mesh");
                meshNode->mesh = RED4ext::RaRef<RED4ext::CMesh>(replacementPath);
            }
        }
    }
}
