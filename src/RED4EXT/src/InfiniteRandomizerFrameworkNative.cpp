#include "InfiniteRandomizerFrameworkNative.h"

#include "RED4ext/Scripting/Utils.hpp"
#include "Red4ext/Red4ext.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/StreamingSector.hpp"
#include "globals.h"
#include "StreamingSectorNodeBuffer.h"
#include "RED4ext/Scripting/Natives/Generated/world/MeshNode.hpp"
#include <RedLib.hpp>

namespace InfiniteRandomizerFramework
{
    void InfiniteRandomizerFrameworkNative::OnSectorPostLoad(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4) {
        RED4ext::Handle<RED4ext::worldStreamingSector> sector;
        RED4ext::GetParameter(aFrame, &sector);
        aFrame->code++;

        g_sdk->logger->Info(g_pHandle, "OnSectorPostLoad called");

        auto* pRTTI = RED4ext::CRTTISystem::Get();
        for (auto& nodes = GetNodes(sector); const auto& node : nodes)
        {
            if (node->GetNativeType()->IsA(pRTTI->GetType("worldMeshNode")))
            {
                g_sdk->logger->Info(g_pHandle, "Found worldMeshNode!");
                const auto meshNode = Red::Cast<RED4ext::worldMeshNode>(node);
                constexpr auto replacementPath = RED4ext::ResourcePath("cyberZ(h)ines\\meshes\\cyberzhines.mesh");
                meshNode->mesh = RED4ext::RaRef<RED4ext::CMesh>(replacementPath);
            }
        }
    }
}
