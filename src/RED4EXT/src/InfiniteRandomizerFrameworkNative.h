#pragma once
#include "RED4ext/Scripting/IScriptable.hpp"
#include "RED4ext/Scripting/Stack.hpp"

namespace InfiniteRandomizerFramework
{
    struct InfiniteRandomizerFrameworkNative : RED4ext::IScriptable
    {
        static void OnSectorPostLoad(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                              int64_t a4);
        RED4ext::CClass* GetNativeType();
    };

}
