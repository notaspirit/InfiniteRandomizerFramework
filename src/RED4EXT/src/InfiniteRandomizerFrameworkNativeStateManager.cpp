#include "InfiniteRandomizerFrameworkNative.h"

#include <fstream>
#include "RED4ext/Scripting/Utils.hpp"
#include "Red4ext/Red4ext.hpp"
#include "DataStructs/Globals.h"
#include <RedLib.hpp>
#include "RED4ext/ResourceDepot.hpp"
#include "RedLogger.h"
#include <RapidJson/document.h>
#include <RapidJson/error/en.h>


namespace fs = std::filesystem;

namespace InfiniteRandomizerFramework
{
    void InfiniteRandomizerFrameworkNative::Initialize(RED4ext::IScriptable *aContext, RED4ext::CStackFrame *aFrame, RED4ext::CString *aOut, int64_t a4) {
        aFrame->code++;
        if (m_initialized)
        {
            return;
        }
        RedLogger::Info("Initializing InfiniteRandomizerFramework Native Systems...");

        m_depot = RED4ext::ResourceDepot::Get();
        m_rttis = RED4ext::CRTTISystem::Get();

        LoadFromDiskInternal();

        m_initialized = true;
    }

    void InfiniteRandomizerFrameworkNative::LoadFromDisk(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, int64_t a4)
    {
        aFrame->code++;
        LoadFromDiskInternal();
    }

    void InfiniteRandomizerFrameworkNative::LoadFromDiskInternal()
    {
        const auto variantPoolDir = std::filesystem::current_path().string() + "\\plugins\\cyber_engine_tweaks\\mods\\InfiniteRandomizerFramework\\data\\variantPools";
        const auto categoryDir = std::filesystem::current_path().string() + "\\plugins\\cyber_engine_tweaks\\mods\\InfiniteRandomizerFramework\\data\\categories";

        m_replacements.clear();

        auto loadedVariantPools = std::make_unique<std::unordered_map<std::string, std::shared_ptr<Replacements>>>();
        auto categoryTypeLookup = std::make_unique<std::unordered_map<std::string, std::string>>();

        // load categories and create replacement pools
        try
        {
            for (const auto& entry : fs::directory_iterator(categoryDir))
            {
                auto entryPath = entry.path().string();
                if (!entryPath.ends_with(".json") || !entry.is_regular_file())
                {
                    continue;
                }

                rapidjson::Document doc;
                std::ifstream fileStream(entryPath);
                std::stringstream buffer;
                buffer << fileStream.rdbuf();
                doc.Parse(buffer.str().c_str());

                if (doc.HasParseError())
                {
                    g_sdk->logger->Warn(g_pHandle, std::format("Failed to load category {} from disk due to a json error.", entryPath).c_str());
                    continue;
                }

                if (!doc.HasMember("name") || !doc["name"].IsString())
                {
                    g_sdk->logger->Warn(g_pHandle, std::format("Invalid Format in {}, category contains no name or is of wrong type.", entryPath).c_str());
                    continue;
                }
                const auto name = doc["name"].GetString();

                if (!doc.HasMember("entries") || !doc["entries"].IsArray())
                {
                    g_sdk->logger->Warn(g_pHandle, std::format("Invalid Format in {}, category contains no entries or is of wrong type.", entryPath).c_str());
                    continue;
                }
                const auto entries = doc["entries"].GetArray();

                // validate type coherence and add to lookup
                auto allEntriesInvalid = false;
                for (auto i = 0; i < entries.Size(); i++)
                {
                    auto& entry = entries[i];
                    if (!entry.HasMember("resourcePath") || !entry["resourcePath"].IsString())
                    {
                        g_sdk->logger->Warn(g_pHandle, std::format("Invalid Format in {} at index {}, category entry contains no resource path or is of wrong type.", entryPath, i).c_str());
                        categoryTypeLookup->erase(name);
                        goto continue_category;
                    }

                    const auto resourcePath = entry["resourcePath"].GetString();
                    const auto extension = fs::path(resourcePath).extension().string();
                    categoryTypeLookup->insert({name, extension});

                    if (categoryTypeLookup->find(name)->second != extension)
                    {
                        g_sdk->logger->Warn(g_pHandle, std::format("Invalid Format in {}, category contains resources of mixed type.", entryPath, i).c_str());
                        categoryTypeLookup->erase(name);
                        goto continue_category;
                    }
                }



                continue_category:
            }
        }
        catch (const std::exception &e)
        {
            g_sdk->logger->Error(g_pHandle, std::format("Failed to load Categories from disk with error: {}", e.what()).c_str());
            return;
        }

        try
        {
            for (const auto& entry : fs::directory_iterator(variantPoolDir))
            {
                // g_sdk->logger->Info(g_pHandle, entry.path().string().c_str());
                auto entryPath = entry.path().string();
                if (!entryPath.ends_with(".json") || !entry.is_regular_file())
                {
                    continue;
                }

                rapidjson::Document doc;
                std::ifstream fileStream(entryPath);
                std::stringstream buffer;
                buffer << fileStream.rdbuf();
                doc.Parse(buffer.str().c_str());

                if (doc.HasParseError())
                {
                    g_sdk->logger->Info(g_pHandle, std::format("Failed to load Variant Pool {} from disk due to a json error.", entryPath).c_str());
                    continue;
                }

                if (!doc.HasMember("enabled") || !doc["enabled"].IsBool())
                {
                    g_sdk->logger->Info(g_pHandle, std::format("Invalid Format in {}, variant Pool contains no enabled state or is of wrong type.", entryPath).c_str());
                    continue;
                }
                if (!doc["enabled"].GetBool())
                {
                    continue;
                }

                if (!doc.HasMember("name") || !doc["name"].IsString())
                {
                    g_sdk->logger->Info(g_pHandle, std::format("Invalid Format in {}, variant Pool contains no name or is of wrong type.", entryPath).c_str());
                    continue;
                }

                if (!doc.HasMember("category") || !doc["category"].IsString())
                {
                    g_sdk->logger->Info(g_pHandle, std::format("Invalid Format in {}, variant Pool contains no category or is of wrong type.", entryPath).c_str());
                    continue;
                }
                const auto category = doc["category"].GetString();

                if (!doc.HasMember("variants") || !doc["variants"].IsArray())
                {
                    g_sdk->logger->Info(g_pHandle, std::format("Invalid Format in {}, variant Pool contains no category or is of wrong type.", entryPath).c_str());
                    continue;
                }
                const auto variants = doc["variants"].GetArray();
                auto i = 0;
                for (const auto& variant : variants)
                {
                    if (!variant.HasMember("resourcePath") || !variant["resourcePath"].IsString())
                    {
                        g_sdk->logger->Info(g_pHandle, std::format("Invalid Format in {} at index {}, variant entry contains no resource path or is of wrong type.", entryPath, i).c_str());
                        continue;
                    }
                    const auto resourcePathString = variant["resourcePath"].GetString();
                    const auto resourcePathHash = RED4ext::ResourcePath(resourcePathString).hash;

                    if (!categoryTypeLookup->contains(resourcePathHash))
                    {
                        const auto resourcePathExt = fs::path(resourcePathString).extension().string();
                        categoryTypeLookup->insert({resourcePathHash, resourcePathExt});
                    }

                    float weight;
                    if (!variant.HasMember("weight") || !variant["weight"].IsFloat())
                    {
                        weight = 1.0f;
                    }
                    else
                    {
                        weight = variant["weight"].GetFloat();
                    }

                    const char* appearance;
                    if (!variant.HasMember("appearance") || !variant["appearance"].IsString())
                    {
                        appearance = "default";
                    }
                    else
                    {
                        appearance = variant["appearance"].GetString();
                    }




                    i++;
                }

            }
        }
        catch (const std::exception &e)
        {
            g_sdk->logger->Error(g_pHandle, std::format("Failed to load Variant Pools from disk with error: {}", e.what()).c_str());
            return;
        }
    }

    std::unordered_map<std::string, Category> InfiniteRandomizerFrameworkNative::LoadCategoriesFromDisk() {
        const auto categoryDir = std::filesystem::current_path().string() + R"(\plugins\cyber_engine_tweaks\mods\InfiniteRandomizerFramework\data\categories)";
        auto parsedCategories = std::unordered_map<std::string, Category>();

        try
        {
        for (const auto& entry : fs::directory_iterator(categoryDir))
        {
            auto entryPath = entry.path().string();
            if (!entryPath.ends_with(".json") || !entry.is_regular_file())
            {
                continue;
            }

            rapidjson::Document doc;
            std::ifstream fileStream(entryPath);
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            doc.Parse(buffer.str().c_str());

            if (doc.HasParseError()) {
                RedLogger::Error(std::format("Failed to parse category file {} with error {}.", entryPath, rapidjson::GetParseError_En(doc.GetParseError())));
            }
        }
        }
        catch (const std::exception &e)
        {
        RedLogger::Error(std::format("Failed to load Categories from disk with error: {}", e.what()));
        return {};
        }
    }

}
