#pragma once
#include "DataStructs/Category.h"
#include "DataStructs/Replacements.h"
#include "RED4ext/ResourceDepot.hpp"
#include "RED4ext/RTTISystem.hpp"
#include "RED4ext/Scripting/IScriptable.hpp"
#include "RED4ext/Scripting/Stack.hpp"

namespace InfiniteRandomizerFramework
{

class InfiniteRandomizerFrameworkNative : RED4ext::IScriptable
{
public:
    static void Initialize(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                          int64_t a4);
    static void LoadFromDisk(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                      int64_t a4);
    static void OnSectorPostLoad(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut,
                          int64_t a4);
    RED4ext::CClass* GetNativeType();
private:
    static inline bool m_initialized = false;
    static inline std::unordered_map<uint64_t, std::unordered_map<RED4ext::CName, Replacements>> m_replacements;
    static inline RED4ext::ResourceDepot* m_depot = std::nullptr_t();
    static inline RED4ext::CRTTISystem* m_rttis = std::nullptr_t();
    static void LoadFromDiskInternal();
    static std::unordered_map<std::string, Category> LoadCategoriesFromDisk();
};

}
