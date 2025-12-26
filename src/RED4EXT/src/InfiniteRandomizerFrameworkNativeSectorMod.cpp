#include <memory>

#include "InfiniteRandomizerFrameworkNative.h"

#include "DataStructs/Globals.h"
#include "DataStructs/StreamingSectorNodeBuffer.h"
#include "RED4ext/Scripting/Natives/Generated/world/BendedMeshNode.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/EntityNode.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/FoliageNode.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/InstancedMeshNode.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/MeshNode.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/StaticDecalNode.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/StreamingSector.hpp"
#include "RED4ext/Scripting/Natives/Generated/world/TerrainMeshNode.hpp"
#include "RED4ext/Scripting/Utils.hpp"
#include "RedLib.hpp"
#include "RedLogger.h"

namespace InfiniteRandomizerFramework {

std::tuple<RED4ext::ResourcePath, RED4ext::CName>
InfiniteRandomizerFrameworkNative::GetRandomEntry(
    const RED4ext::ResourcePath &resourcePath,
    const RED4ext::CName &appearance) {

    const auto replacement = m_replacements.at(resourcePath);

    auto& anyReplacements = replacement.at(g_anyAppearance);
    float randWeight;

    if (replacement.contains(appearance)) {
        const auto& appReplacements = replacement.at(appearance);

        randWeight = m_rng.getFloat(anyReplacements->weights->at(0) +
                                               appReplacements->weights->at(0));

        for (auto i = 1; i < appReplacements->weights->size(); i++) {
            if (randWeight <= appReplacements->weights->at(i)) {
                return std::tuple(appReplacements->resourcePaths->at(i - 1),
                    appReplacements->appNames->at(i - 1));
              break;
            }
        }
    }
    else {
        randWeight = m_rng.getFloat(anyReplacements->weights->at(0));
    }

    for (auto i = 1; i < anyReplacements->weights->size(); i++) {
        if (randWeight <= anyReplacements->weights->at(i)) {
            return std::tuple(anyReplacements->resourcePaths->at(i - 1),
                              anyReplacements->appNames->at(i - 1));
            break;
        }
    }
}

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

            if (!m_replacements.contains(meshNode->mesh.path)) {
                continue;
            }
            
            auto replacementValues = GetRandomEntry(meshNode->mesh.path, meshNode->meshAppearance);
            meshNode->mesh = RED4ext::RaRef<RED4ext::CMesh>(replacementValues._Myfirst._Val);
            meshNode->meshAppearance = replacementValues._Get_rest()._Myfirst._Val;
        }
        else if (node->GetNativeType()->IsA(m_rttis->GetType("worldInstancedMeshNode"))) {
            const auto instancedMeshNode = Red::Cast<RED4ext::worldInstancedMeshNode>(node);

            if (!m_replacements.contains(instancedMeshNode->mesh.path)) {
                continue;
            }

            auto replacementValues = GetRandomEntry(instancedMeshNode->mesh.path, instancedMeshNode->meshAppearance);
            instancedMeshNode->mesh = RED4ext::RaRef<RED4ext::CMesh>(replacementValues._Myfirst._Val);
            instancedMeshNode->meshAppearance = replacementValues._Get_rest()._Myfirst._Val;
        }
        else if (node->GetNativeType()->IsA(m_rttis->GetType("worldBendedMeshNode"))) {
            const auto bendedMeshNode = Red::Cast<RED4ext::worldBendedMeshNode>(node);

            if (!m_replacements.contains(bendedMeshNode->mesh.path)) {
                continue;
            }

            auto replacementValues = GetRandomEntry(bendedMeshNode->mesh.path, bendedMeshNode->meshAppearance);
            bendedMeshNode->mesh = RED4ext::RaRef<RED4ext::CMesh>(replacementValues._Myfirst._Val);
            bendedMeshNode->meshAppearance = replacementValues._Get_rest()._Myfirst._Val;
        }
        else if (node->GetNativeType()->IsA(m_rttis->GetType("worldFoliageNode"))) {
            const auto foliageMeshNode = Red::Cast<RED4ext::worldFoliageNode>(node);

            if (!m_replacements.contains(foliageMeshNode->mesh.path)) {
                continue;
            }

            auto replacementValues = GetRandomEntry(foliageMeshNode->mesh.path, foliageMeshNode->meshAppearance);
            foliageMeshNode->mesh = RED4ext::RaRef<RED4ext::CMesh>(replacementValues._Myfirst._Val);
            foliageMeshNode->meshAppearance = replacementValues._Get_rest()._Myfirst._Val;
        }
        else if (node->GetNativeType()->IsA(m_rttis->GetType("worldTerrainMeshNode"))) {
            const auto terrainMeshNode = Red::Cast<RED4ext::worldTerrainMeshNode>(node);

            if (!m_replacements.contains(terrainMeshNode->meshRef.path)) {
                continue;
            }

            auto replacementValues = GetRandomEntry(terrainMeshNode->meshRef.path, g_anyAppearance);
            terrainMeshNode->meshRef = RED4ext::RaRef<RED4ext::CMesh>(replacementValues._Myfirst._Val);
        }
        else if (node->GetNativeType()->IsA(m_rttis->GetType("worldEntityNode"))) {
            const auto entityNode = Red::Cast<RED4ext::worldEntityNode>(node);

            if (!m_replacements.contains(entityNode->entityTemplate.path)) {
                continue;
            }

            auto replacementValues = GetRandomEntry(entityNode->entityTemplate.path, entityNode->appearanceName);
            entityNode->entityTemplate = RED4ext::RaRef<RED4ext::ent::EntityTemplate>(replacementValues._Myfirst._Val);
            entityNode->appearanceName = replacementValues._Get_rest()._Myfirst._Val;
        }
        else if (node->GetNativeType()->IsA(m_rttis->GetType("worldStaticDecalNode"))) {
            const auto decalNode = Red::Cast<RED4ext::worldStaticDecalNode>(node);

            if (!m_replacements.contains(decalNode->material.path)) {
                continue;
            }

            auto replacementValues = GetRandomEntry(decalNode->material.path, g_anyAppearance);
            decalNode->material = RED4ext::RaRef<RED4ext::IMaterial>(replacementValues._Myfirst._Val);
        }
    }
}
}
