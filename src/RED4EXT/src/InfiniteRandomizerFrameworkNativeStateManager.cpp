#include "InfiniteRandomizerFrameworkNative.h"

#include <fstream>
#include "RED4ext/Scripting/Utils.hpp"
#include "Red4ext/Red4ext.hpp"
#include "DataStructs/Globals.h"
#include <RedLib.hpp>
#include <unordered_set>

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
        m_replacements.clear();

        auto loadedVariantPools = std::make_unique<std::unordered_map<std::string, std::shared_ptr<Replacements>>>();
        auto categoryTypeLookup = std::make_unique<std::unordered_map<std::string, std::string>>();

        auto categories = LoadCategoriesFromDisk();
        auto variantPools = LoadVariantPoolsFromDisk();


    }

    std::unordered_map<std::string, Category> InfiniteRandomizerFrameworkNative::LoadCategoriesFromDisk() {
        const auto categoryDir = std::filesystem::current_path().string() + R"(\plugins\cyber_engine_tweaks\mods\InfiniteRandomizerFramework\data\categories)";
        auto parsedCategories = std::unordered_map<std::string, Category>();

        try
        {
        for (const auto& categoryFile : fs::directory_iterator(categoryDir))
        {
            auto entryPath = categoryFile.path().string();
            if (!entryPath.ends_with(".json") || !categoryFile.is_regular_file())
            {
                continue;
            }

            auto category = Category();
            std::string name;

            rapidjson::Document doc;
            std::ifstream fileStream(entryPath);
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            doc.Parse(buffer.str().c_str());

            if (doc.HasParseError()) {
                RedLogger::Error(std::format("Failed to parse category file {} with error {}.", entryPath, rapidjson::GetParseError_En(doc.GetParseError())));
                continue;
            }

            if (!doc.IsObject()) {
                RedLogger::Error(std::format("Category file {} is malformed: root is not of type object.", entryPath));
                continue;
            }

            if (!doc.HasMember("name")) {
                RedLogger::Error(std::format("Category file {} is malformed: missing property `name`.", entryPath));
                continue;
            }

            if (!doc["name"].IsString()) {
                RedLogger::Error(std::format("Category file {} is malformed: property `name` is not of type string.", entryPath));
                continue;
            }

            name = doc["name"].GetString();

            if (!doc.HasMember("entries")) {
                RedLogger::Error(std::format("Category file {} is malformed: missing property `entries`.", entryPath));
                continue;
            }

            if (!doc["entries"].IsArray()) {
                RedLogger::Error(std::format("Category file {} is malformed: property `entries` is not of type array.", entryPath));
                continue;
            }

            auto entries = doc["entries"].GetArray();
            auto i = 0;
            for (const auto& entry : entries) {
                if (!entry.IsObject()) {
                    RedLogger::Error(std::format("Category entry {} in {} is malformed: root is not of type object.", i, entryPath));
                    continue;
                }

                if (!entry.HasMember("resourcePath")) {
                    RedLogger::Error(std::format("Category entry {} in {} is malformed: missing property `resourcePath`.", i, entryPath));
                    continue;
                }

                if (!doc["resourcePath"].IsString()) {
                    RedLogger::Error(std::format("Category entry {} in {} is malformed: property `resourcePath` is not of type string.", i, entryPath));
                    continue;
                }

                const auto resourcePathString = entry["resourcePath"].GetString();
                const auto redResourcePath = RED4ext::ResourcePath(resourcePathString);
                const auto extension = fs::path(resourcePathString).extension().string();

                if (category.extension.empty()) {
                    category.extension = extension;
                }

                if (category.extension != extension) {
                    RedLogger::Error(std::format("Category file {} is malformed: entries contains mixed resource types", entryPath));
                    goto breakCatLoop;
                }

                auto catEntry = CategoryEntry();
                catEntry.resourcePath = redResourcePath;

                if (entry.HasMember("appearance")) {
                    if (doc["appearance"].IsString()) {
                        catEntry.appearance = doc["appearance"].GetString();
                    }
                    else {
                        RedLogger::Warning(std::format("Category entry {} in {} is malformed: property `appearance` is not of type string, using default.", i, entryPath));
                        catEntry.appearance = g_anyAppearance;
                    }
                }
                else {
                    catEntry.appearance = g_anyAppearance;
                }

                category.entries.push_back(catEntry);

                i++;
            }

            if (parsedCategories.contains(name)) {
                RedLogger::Error(std::format("Failed to load category {} at {}: category with conflicting name exists.", name, entryPath));
            }
            else {
                parsedCategories[name] = category;
            }
            breakCatLoop:
                ;
        }
        }
        catch (const std::exception &e)
        {
        RedLogger::Error(std::format("Failed to load Categories from disk with error: {}", e.what()));
        return {};
        }
        return parsedCategories;
    }

    std::unordered_map<std::string, VariantPool> LoadVariantPoolsFromDisk() {
        const auto variantPoolDir = std::filesystem::current_path().string() + R"(\plugins\cyber_engine_tweaks\mods\InfiniteRandomizerFramework\data\variantPools)";
        std::unordered_map<std::string, VariantPool> parsedPools;

        try
        {
        for (const auto& poolFile : fs::directory_iterator(variantPoolDir)) {
            auto entryPath = poolFile.path().string();
            if (!entryPath.ends_with(".json") || !poolFile.is_regular_file())
            {
                continue;
            }

            auto pool = VariantPool();
            std::string name;

            rapidjson::Document doc;
            std::ifstream fileStream(entryPath);
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            doc.Parse(buffer.str().c_str());

            if (doc.HasParseError()) {
                RedLogger::Error(std::format("Failed to parse variant pool file {} with error {}.", entryPath, rapidjson::GetParseError_En(doc.GetParseError())));
                continue;
            }

            if (!doc.IsObject()) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: root is not of type object.", entryPath));
                continue;
            }

            if (!doc.HasMember("enabled")) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: missing property `enabled`.", entryPath));
                continue;
            }

            if (!doc["enabled"].IsBool()) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: property `enabled` is not of type bool.", entryPath));
                continue;
            }

            if (!doc["enabled"].GetBool()) {
                continue;
            }

            if (!doc.HasMember("name")) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: missing property `name`.", entryPath));
                continue;
            }

            if (!doc["name"].IsString()) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: property `name` is not of type string.", entryPath));
                continue;
            }

            name = doc["name"].GetString();

            if (!doc.HasMember("category")) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: missing property `category`.", entryPath));
                continue;
            }

            if (!doc["category"].IsString()) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: property `category` is not of type string.", entryPath));
                continue;
            }

            pool.category = doc["category"].GetString();

            if (!doc.HasMember("variants")) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: missing property `variants`.", entryPath));
                continue;
            }

            if (!doc["variants"].IsArray()) {
                RedLogger::Error(std::format("Variant pool file {} is malformed: property `variants` is not of type array.", entryPath));
                continue;
            }

            auto variantArray = doc["variants"].GetArray();
            auto i = 0;
            for (const auto& entry : variantArray) {
                if (!entry.IsObject()) {
                    RedLogger::Error(std::format("Variant pool entry {} in {} is malformed: root is not of type object.", i, entryPath));
                    continue;
                }

                if (!entry.HasMember("resourcePath")) {
                    RedLogger::Error(std::format("Variant pool entry {} in {} is malformed: missing property `resourcePath`.", i, entryPath));
                    continue;
                }

                if (!doc["resourcePath"].IsString()) {
                    RedLogger::Error(std::format("Variant pool entry {} in {} is malformed: property `resourcePath` is not of type string.", i, entryPath));
                    continue;
                }

                const auto resourcePathString = entry["resourcePath"].GetString();
                const auto redResourcePath = RED4ext::ResourcePath(resourcePathString);
                const auto extension = fs::path(resourcePathString).extension().string();

                if (pool.extension.empty()) {
                    pool.extension = extension;
                }

                if (pool.extension != extension) {
                    RedLogger::Error(std::format("Category file {} is malformed: entries contains mixed resource types", entryPath));
                    goto breakPoolLoop;
                }

                auto variant = VariantPoolEntry();
                variant.resourcePath = redResourcePath;

                if (entry.HasMember("weight")) {
                    if (doc["weight"].IsDouble()) {
                        variant.weight = doc["weight"].GetDouble();
                    }
                    else {
                        RedLogger::Warning(std::format("Variant pool entry {} in {} is malformed: property `weight` is not of type number, using default.", i, entryPath));
                        variant.weight = 1;
                    }
                }
                else {
                    variant.weight = 1;
                }

                if (entry.HasMember("appearance")) {
                    if (doc["appearance"].IsString()) {
                        variant.appearance = doc["appearance"].GetString();
                    }
                    else {
                        RedLogger::Warning(std::format("Variant pool entry {} in {} is malformed: property `appearance` is not of type string, using default.", i, entryPath));
                        variant.appearance = "default";
                    }
                }
                else {
                    variant.appearance = "default";
                }

                pool.entries.push_back(variant);
            }

            if (parsedPools.contains(name)) {
                RedLogger::Error(std::format("Failed to load variant pool {} at {}: variant pool with conflicting name exists.", name, entryPath));
            }
            else {
                parsedPools[name] = pool;
            }
            breakPoolLoop:
                ;
        }
        }
        catch (const std::exception &e)
        {
        RedLogger::Error(std::format("Failed to load Variant Pools from disk with error: {}", e.what()));
        return {};
        }
        return parsedPools;
    }

}
