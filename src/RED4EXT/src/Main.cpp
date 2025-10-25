#include <RED4ext/RED4ext.hpp>
#include "InfiniteRandomizerFrameworkNative.h"
#include "DataStructs/Globals.h"

namespace InfiniteRandomizerFramework
{
    RED4ext::PluginHandle g_pHandle;
    const RED4ext::Sdk* g_sdk;

    RED4ext::TTypedClass<InfiniteRandomizerFrameworkNative> customControllerClass(
    "InfiniteRandomizerFrameworkNative");

    RED4ext::CClass* InfiniteRandomizerFrameworkNative::GetNativeType()
    {
        return &customControllerClass;
    }

    RED4EXT_C_EXPORT void RED4EXT_CALL RegisterTypes()
    {
        RED4ext::CNamePool::Add("InfiniteRandomizerFrameworkNative");

        customControllerClass.flags = {.isNative = true};
        RED4ext::CRTTISystem::Get()->RegisterType(&customControllerClass);
    }

    RED4EXT_C_EXPORT void RED4EXT_CALL PostRegisterTypes()
    {
        const auto rtti = RED4ext::CRTTISystem::Get();
        const auto scriptable = rtti->GetClass("IScriptable");
        customControllerClass.parent = scriptable;

        const auto onSectorPostLoad =
            RED4ext::CClassStaticFunction::Create(&customControllerClass, "OnSectorPostLoad", "OnSectorPostLoad",
            &InfiniteRandomizerFrameworkNative::OnSectorPostLoad, {.isNative = true, .isStatic = true});

        onSectorPostLoad->AddParam("handle:worldStreamingSector", "sector");
        customControllerClass.RegisterFunction(onSectorPostLoad);

        const auto initialize =
            RED4ext::CClassStaticFunction::Create(&customControllerClass, "Initialize", "Initialize",
            &InfiniteRandomizerFrameworkNative::Initialize, {.isNative = true, .isStatic = true});

        customControllerClass.RegisterFunction(initialize);
    }

    RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason, const RED4ext::Sdk* aSdk)
    {
        switch (aReason)
        {
            case RED4ext::EMainReason::Load:
            {
                g_pHandle = aHandle;
                g_sdk = aSdk;

                RED4ext::CRTTISystem::Get()->AddRegisterCallback(RegisterTypes);
                RED4ext::CRTTISystem::Get()->AddPostRegisterCallback(PostRegisterTypes);

                g_sdk->logger->Info(g_pHandle, "InfiniteRandomizerNative loaded");
                break;
            }
            case RED4ext::EMainReason::Unload:
            {
                break;
            }
        }

        return true;
    }

    RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
    {
        aInfo->name = L"InfiniteRandomizerFrameworkNative";
        aInfo->author = L"sprt_";
        aInfo->version = RED4EXT_SEMVER(1, 0, 0);
        aInfo->runtime = RED4EXT_RUNTIME_LATEST;
        aInfo->sdk = RED4EXT_SDK_LATEST;
    }

    RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
    {
        return RED4EXT_API_VERSION_LATEST;
    }
}