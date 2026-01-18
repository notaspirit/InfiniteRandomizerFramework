#include "InfiniteRandomizerFrameworkNative.h"

#include <fstream>
#include <ranges>

#include "RED4ext/Scripting/Utils.hpp"
#include "Red4ext/Red4ext.hpp"
#include "DataStructs/Globals.h"
#include <RedLib.hpp>
#include <unordered_set>

#include "RED4ext/ResourceDepot.hpp"
#include "RedLogger.h"
#include <RapidJson/document.h>
#include <RapidJson/error/en.h>
#include "xxhash64.h"

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

        m_rng = FastRNG();
        m_rng.state = std::chrono::system_clock::now().time_since_epoch().count();

        LoadFromDiskInternal();

        m_initialized = true;
        RedLogger::Info("Initialized InfiniteRandomizerFramework Native");
    }

    void InfiniteRandomizerFrameworkNative::LoadFromDisk(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, RED4ext::CString* aOut, int64_t a4)
    {
        aFrame->code++;
        LoadFromDiskInternal();
    }

    void InfiniteRandomizerFrameworkNative::LoadFromDiskInternal()
    {
        RedLogger::Info("Loading State From Disk...");

        m_replacements.clear();

        auto categories = LoadCategoriesFromDisk();
        auto variantPools = LoadVariantPoolsFromDisk();

        RedLogger::Info(std::format("Parsed {} categories", categories.size()));
        RedLogger::Info(std::format("Parsed {} variant pools", variantPools.size()));

        // string represents the xxhash64 as a list of all categories that the given replacement contains, separated via a _
        std::unordered_map<std::string, std::shared_ptr<Replacements>> replacementMap;

        RedLogger::Info("Loading Variant Pools...");

        for (const auto& pool : variantPools) {
            if (!categories.contains(pool.second.category)) {
                RedLogger::Error(std::format("Failed to load variant pool {}: target category {} does not exist.", pool.first, pool.second.category));
                continue;
            }

            Category& targetCat = categories.at(pool.second.category);
            if (targetCat.extension != pool.second.extension) {
                RedLogger::Error(std::format("Failed to load variant pool {}: target category {} type ({}), does not match variant pool type ({}).", pool.first, pool.second.category, targetCat.extension, pool.second.extension));
                continue;
            }

            std::shared_ptr<Replacements> replacement;
            auto catNameHash = XXHash64::hash(pool.second.category.data(), pool.second.category.size(), 0);
            auto catNameHashStr = std::to_string(catNameHash);
            if (replacementMap.contains(catNameHashStr)) {
                replacement = replacementMap.at(catNameHashStr);
            }
            else {
                replacement = std::make_shared<Replacements>();
                replacement->weights = std::make_unique<std::vector<float>>();
                replacement->appNames = std::make_unique<std::vector<RED4ext::CName>>();
                replacement->resourcePaths = std::make_unique<std::vector<RED4ext::ResourcePath>>();
                replacement->weights->push_back(0);
            }

            for (const auto& poolEntry : pool.second.entries) {
                replacement->weights->at(0) += poolEntry.weight;
                replacement->weights->push_back(poolEntry.weight);
                replacement->appNames->push_back(RED4ext::CName(poolEntry.appearance.c_str()));
                replacement->resourcePaths->push_back(poolEntry.resourcePath);
            }

            if (!replacementMap.contains(catNameHashStr)) {
                replacementMap.insert(std::make_pair(catNameHashStr, replacement));
            }
        }

        RedLogger::Info("Loading Categories...");

        std::unordered_map<uint64_t, std::unordered_map<RED4ext::CName, std::string>> addedCategories;

        for (const auto& cat : categories) {
            auto catNameHash = XXHash64::hash(cat.first.data(), cat.first.size(), 0);
            auto catNameHashStr = std::to_string(catNameHash);

            if (!replacementMap.contains(catNameHashStr)) {
                continue;
            }

            for (const auto& catEntry : cat.second.entries) {
                if (addedCategories.contains(catEntry.resourcePath.hash)) {
                    auto& existingAppMap = addedCategories.at(catEntry.resourcePath.hash);
                    if (existingAppMap.contains(catEntry.appearance)) {
                        auto& existingCatName = existingAppMap.at(catEntry.appearance);
                        auto& existingReplacement = replacementMap.at(existingCatName);
                        existingCatName = std::format("{}_{}", existingCatName, catNameHashStr);

                        auto newReplacement = std::make_shared<Replacements>();
                        std::ranges::copy(*existingReplacement->weights, newReplacement->weights->begin());
                        std::ranges::copy(*existingReplacement->appNames, newReplacement->appNames->begin());
                        std::ranges::copy(*existingReplacement->resourcePaths, newReplacement->resourcePaths->begin());

                        newReplacement->weights->insert(newReplacement->weights->end(), *existingReplacement->weights->begin() + 1);
                        newReplacement->appNames->insert(newReplacement->appNames->end(), *existingReplacement->appNames->begin());
                        newReplacement->resourcePaths->insert(newReplacement->resourcePaths->end(), *existingReplacement->resourcePaths->begin());

                        newReplacement->weights->at(0) = 0.0f;
                        for (const auto& weightEntry : *newReplacement->weights) {
                            newReplacement->weights->at(0) += weightEntry;
                        }

                        replacementMap.at(catNameHashStr) = std::move(newReplacement);
                    }
                    else {
                        existingAppMap[catEntry.appearance] = catNameHashStr;

                        m_replacements.at(catEntry.resourcePath.hash).insert({catEntry.appearance, replacementMap.at(catNameHashStr)});
                    }
                }
                else {
                    auto innerNameMap = std::unordered_map<RED4ext::CName, std::string>();
                    innerNameMap.insert({catEntry.appearance, catNameHashStr});
                    addedCategories.insert({catEntry.resourcePath.hash, innerNameMap});

                    auto innerRepMap = std::unordered_map<RED4ext::CName, std::shared_ptr<Replacements>>();
                    innerRepMap.insert({catEntry.appearance, replacementMap.at(catNameHashStr)});
                    m_replacements.insert({catEntry.resourcePath.hash, innerRepMap});
                }
            }
        }

        std::unordered_map<uint64_t, bool> processedSharedPtr;
        for (auto &val : m_replacements | std::views::values) {
            for (auto &valInner : val | std::views::values) {
                if (processedSharedPtr.contains((uint64_t)valInner.get()))
                    continue;

                processedSharedPtr.insert({(uint64_t)valInner.get(), true});

                if (valInner->weights->size() < 3)
                    continue;

                for (int i = 2; i < valInner->weights->size(); i++)
                    valInner->weights->at(i) += valInner->weights->at(i - 1);
            }

            if (!val.contains(g_anyAppearance) && !val.empty()) {
                auto anyRep = std::make_shared<Replacements>();
                anyRep->weights = std::make_unique<std::vector<float>>();
                anyRep->appNames = std::make_unique<std::vector<RED4ext::CName>>();
                anyRep->resourcePaths = std::make_unique<std::vector<RED4ext::ResourcePath>>();

                anyRep->weights->push_back(0);

                val.insert({g_anyAppearance, anyRep});
            }
        }

        for (auto it = m_replacements.begin(); it != m_replacements.end(); ) {
            if (it->second.empty()) {
                it = m_replacements.erase(it);
            } else {
                ++it;
            }
        }

        RedLogger::Info("Finished Loading");
    }

    std::filesystem::path GetExeDir() {
        wchar_t buffer[MAX_PATH + 1];

        DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (len == ERROR_INSUFFICIENT_BUFFER)
            throw ERROR_INSUFFICIENT_BUFFER;

        return std::filesystem::path(buffer).parent_path();
    }

    std::unordered_map<std::string, Category> InfiniteRandomizerFrameworkNative::LoadCategoriesFromDisk() {
        std::string categoryDir;
        try {
            categoryDir = GetExeDir().string() + R"(\plugins\cyber_engine_tweaks\mods\InfiniteRandomizerFramework\data\categories)";
        }
        catch (const std::exception& e) {
            RedLogger::Error(std::format("Failed to get executable directory. Cannot load Categories."));
            return {};
        }

        auto parsedCategories = std::unordered_map<std::string, Category>();

        std::size_t count = std::distance(
            fs::directory_iterator(categoryDir),
            fs::directory_iterator{}
        );

        RedLogger::Info(std::format("Found {} category files", count));

        try
        {
        for (const auto& categoryFile : fs::directory_iterator(categoryDir))
        {
            auto entryPath = categoryFile.path().string();
            auto displayPath = categoryFile.path().filename().string();
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

            RedLogger::Info(std::format("Loading category {}", displayPath));

            if (doc.HasParseError()) {
                RedLogger::Error(std::format("Failed to parse category file with error {}.", rapidjson::GetParseError_En(doc.GetParseError())));
                continue;
            }

            if (!doc.IsObject()) {
                RedLogger::Error("Category file is malformed: root is not of type object.");
                continue;
            }

            if (!doc.HasMember("name")) {
                RedLogger::Error("Category file is malformed: missing property `name`.");
                continue;
            }

            if (!doc["name"].IsString()) {
                RedLogger::Error("Category file is malformed: property `name` is not of type string.");
                continue;
            }

            name = doc["name"].GetString();

            if (!doc.HasMember("entries")) {
                RedLogger::Error("Category file is malformed: missing property `entries`.");
                continue;
            }

            if (!doc["entries"].IsArray()) {
                RedLogger::Error("Category file is malformed: property `entries` is not of type array.");
                continue;
            }

            auto entries = doc["entries"].GetArray();
            auto i = -1;
            for (const auto& entry : entries) {
                i++;
                if (!entry.IsObject()) {
                    RedLogger::Warning(std::format("Category entry at {} is malformed: root is not of type object.", i));
                    continue;
                }

                if (!entry.HasMember("resourcePath")) {
                    RedLogger::Warning(std::format("Category entry at {} is malformed: missing property `resourcePath`.", i));
                    continue;
                }

                if (!entry["resourcePath"].IsString()) {
                    RedLogger::Warning(std::format("Category entry at {} is malformed: property `resourcePath` is not of type string.", i));
                    continue;
                }

                const auto resourcePathString = entry["resourcePath"].GetString();
                const auto redResourcePath = RED4ext::ResourcePath(resourcePathString);
                const auto extension = fs::path(resourcePathString).extension().string();

                if (category.extension.empty()) {
                    category.extension = extension;
                }

                if (category.extension != extension) {
                    RedLogger::Error("Category file is malformed: entries contains mixed resource types");
                    goto breakCatLoop;
                }

                auto catEntry = CategoryEntry();
                catEntry.resourcePath = redResourcePath;

                if (entry.HasMember("appearance")) {
                    if (entry["appearance"].IsString()) {
                        catEntry.appearance = entry["appearance"].GetString();
                    }
                    else {
                        RedLogger::Warning(std::format("Category entry at {} is malformed: property `appearance` is not of type string, using default.", i));
                        catEntry.appearance = g_anyAppearance;
                    }
                }
                else {
                    catEntry.appearance = g_anyAppearance;
                }

                category.entries.push_back(catEntry);
            }

            if (parsedCategories.contains(name)) {
                RedLogger::Error("Failed to load category: category with conflicting name exists.");
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

    std::unordered_map<std::string, VariantPool> InfiniteRandomizerFrameworkNative::LoadVariantPoolsFromDisk() {
        std::string variantPoolDir;
        try {
            variantPoolDir = GetExeDir().string() + R"(\plugins\cyber_engine_tweaks\mods\InfiniteRandomizerFramework\data\variantPools)";
        }
        catch (const std::exception& e) {
            RedLogger::Error(std::format("Failed to get executable directory. Cannot load Variant Pools."));
            return {};
        }

        std::unordered_map<std::string, VariantPool> parsedPools;

        std::size_t count = std::distance(
        fs::directory_iterator(variantPoolDir),
        fs::directory_iterator{}
        );

        RedLogger::Info(std::format("Found {} variant pool files", count));

        try
        {
        for (const auto& poolFile : fs::directory_iterator(variantPoolDir)) {
            auto entryPath = poolFile.path().string();
            auto displayPath = poolFile.path().filename().string();
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

            RedLogger::Info(std::format("Loading variant pool {}", displayPath));

            if (doc.HasParseError()) {
                RedLogger::Error(std::format("Failed to parse variant pool file with error {}.", rapidjson::GetParseError_En(doc.GetParseError())));
                continue;
            }

            if (!doc.IsObject()) {
                RedLogger::Error("Variant pool file is malformed: root is not of type object.");
                continue;
            }

            if (!doc.HasMember("enabled")) {
                RedLogger::Error("Variant pool file is malformed: missing property `enabled`.");
                continue;
            }

            if (!doc["enabled"].IsBool()) {
                RedLogger::Error("Variant pool file is malformed: property `enabled` is not of type bool.");
                continue;
            }

            if (!doc["enabled"].GetBool()) {
                RedLogger::Info("Variant pool is disabled.");
                continue;
            }

            if (!doc.HasMember("name")) {
                RedLogger::Error("Variant pool file is malformed: missing property `name`.");
                continue;
            }

            if (!doc["name"].IsString()) {
                RedLogger::Error("Variant pool file is malformed: property `name` is not of type string.");
                continue;
            }

            name = doc["name"].GetString();

            if (!doc.HasMember("category")) {
                RedLogger::Error("Variant pool file is malformed: missing property `category`.");
                continue;
            }

            if (!doc["category"].IsString()) {
                RedLogger::Error("Variant pool files is malformed: property `category` is not of type string.");
                continue;
            }

            pool.category = doc["category"].GetString();

            if (!doc.HasMember("variants")) {
                RedLogger::Error("Variant pool file is malformed: missing property `variants`.");
                continue;
            }

            if (!doc["variants"].IsArray()) {
                RedLogger::Error("Variant pool file is malformed: property `variants` is not of type array.");
                continue;
            }

            auto variantArray = doc["variants"].GetArray();
            auto i = -1;
            for (const auto& entry : variantArray) {
                i++;
                if (!entry.IsObject()) {
                    RedLogger::Error(std::format("Variant pool entry at {} is malformed: root is not of type object.", i));
                    continue;
                }

                if (!entry.HasMember("resourcePath")) {
                    RedLogger::Error(std::format("Variant pool entry at {} is malformed: missing property `resourcePath`.", i));
                    continue;
                }

                if (!entry["resourcePath"].IsString()) {
                    RedLogger::Error(std::format("Variant pool entry at {} is malformed: property `resourcePath` is not of type string.", i));
                    continue;
                }

                const auto resourcePathString = entry["resourcePath"].GetString();
                const auto redResourcePath = RED4ext::ResourcePath(resourcePathString);
                if (!m_depot->ResourceExists(redResourcePath)) {
                    RedLogger::Error(std::format("Variant pool entry at {} is invalid: property `resourcePath` does not point to a valid resource.", i));
                    continue;
                }
                const auto extension = fs::path(resourcePathString).extension().string();

                if (pool.extension.empty()) {
                    pool.extension = extension;
                }

                if (pool.extension != extension) {
                    RedLogger::Error("Category file is malformed: entries contains mixed resource types");
                    goto breakPoolLoop;
                }

                auto variant = VariantPoolEntry();
                variant.resourcePath = redResourcePath;

                if (entry.HasMember("weight")) {
                    if (entry["weight"].IsNumber()) {
                        auto weight = entry["weight"].GetFloat();
                        if (weight <= 0.0f) {
                            RedLogger::Warning(std::format("Variant pool entry at {} is malformed: property `weight` must be bigger than 0, using default.", i));
                            variant.weight = 1.0f;
                        }
                        else {
                            variant.weight = weight;
                        }
                    }
                    else {
                        RedLogger::Warning(std::format("Variant pool entry at {} is malformed: property `weight` is not of type number, using default.", i));
                        variant.weight = 1.0f;
                    }
                }
                else {
                    variant.weight = 1.0f;
                }

                if (entry.HasMember("appearance")) {
                    if (entry["appearance"].IsString()) {
                        variant.appearance = entry["appearance"].GetString();
                    }
                    else {
                        RedLogger::Warning(std::format("Variant pool entry at {} is malformed: property `appearance` is not of type string, using default.", i));
                        variant.appearance = "default";
                    }
                }
                else {
                    variant.appearance = "default";
                }
                pool.entries.push_back(variant);
            }

            if (parsedPools.contains(name)) {
                RedLogger::Error("Failed to load variant pool: variant pool with conflicting name exists.");
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
