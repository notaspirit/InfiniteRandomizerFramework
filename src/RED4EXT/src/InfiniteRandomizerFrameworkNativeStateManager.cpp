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

        /*
        int testSize = 10000;
        int max = 100;
        std::vector<int> testResults(max);
        for (int i = 0; i < testSize; i++)
        {
            testResults[m_rng.getInt32(max, 0)]++;
        }

        std::string csv = "Value, Frequency\n";
        for (int i = 0; i < max; i++) {
            csv += std::format("{},{}\n", i, (float)testResults[i] / (float)testSize);
        }

        RedLogger::Debug(csv);
        */

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
        RedLogger::Debug("Loading State From Disk");

        m_replacements.clear();

        auto categories = LoadCategoriesFromDisk();
        auto variantPools = LoadVariantPoolsFromDisk();

        RedLogger::Debug("Loaded " + std::to_string(categories.size()) + " categories from disk");
        RedLogger::Debug("Loaded " + std::to_string(variantPools.size()) + " pools from disk");

        // string represents the xxhash64 as a list of all categories that the given replacement contains, separated via a _
        std::unordered_map<std::string, std::shared_ptr<Replacements>> replacementMap;

        RedLogger::Debug("Parsed Categories and Variant Pools. Loading Variant Pools...");

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

        int failed = 0;
        for (auto &val : replacementMap | std::views::values) {
            for (auto rs : *val->resourcePaths) {
                if (!m_depot->ResourceExists(rs)) {
                    failed++;
                }
            }

            for (auto w : *val->weights) {
                RedLogger::Debug(std::to_string(w));
            }
        }
        RedLogger::Debug(std::format("{} resource paths are invalid in replacementMap", failed));

        RedLogger::Debug("Loaded Variant Pools. Loading Categories...");

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

        int failedFinal = 0;
        RedLogger::Debug("After Merging Cats.");
        for (auto &val : m_replacements | std::views::values) {
            for (auto &valInner : val | std::views::values) {
                for (auto &rs : *valInner->resourcePaths) {
                    if (!m_depot->ResourceExists(rs)) {
                        failedFinal++;
                    }
                }

                for (auto w : *valInner->weights) {
                    RedLogger::Debug(std::to_string(w));
                }
            }
        }
        RedLogger::Debug(std::format("{} resource paths are invalid in final replacements", failedFinal));

        RedLogger::Debug("Loaded Categories. Recalculating weights...");

        std::unordered_map<uint64_t, bool> processedSharedPtr;
        for (auto &val : m_replacements | std::views::values) {
            for (auto &valInner : val | std::views::values) {
                if (processedSharedPtr.contains((uint64_t)valInner.get())) {
                    RedLogger::Debug(std::format("Already Processed {}", (uint64_t)valInner.get()));
                    continue;
                }

                processedSharedPtr.insert({(uint64_t)valInner.get(), true});

                if (valInner->weights->size() < 3)
                    continue;

                for (int i = 0; i < valInner->weights->size(); i++) {
                    RedLogger::Debug(std::format("Before {}: {:.2f}", i, valInner->weights->at(i)));
                }

                for (int i = 0; i < valInner->weights->size(); i++) {
                    if (i == 0 || i == 1) {
                        continue;
                    }
                    valInner->weights->at(i) += valInner->weights->at(i - 1);
                    RedLogger::Debug(std::format("After {}: {:.2f}", i, valInner->weights->at(i)));
                }
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

        RedLogger::Debug("Finished Loading");
    }

    std::unordered_map<std::string, Category> InfiniteRandomizerFrameworkNative::LoadCategoriesFromDisk() {
        const auto categoryDir = std::filesystem::current_path().string() + R"(\plugins\cyber_engine_tweaks\mods\InfiniteRandomizerFramework\data\categories)";
        auto parsedCategories = std::unordered_map<std::string, Category>();

        std::size_t count = std::distance(
            fs::directory_iterator(categoryDir),
            fs::directory_iterator{}
        );

        RedLogger::Debug("Found " + std::to_string(count) + " categories");

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
            auto i = 0; // TODO: REWORK INDEXING
            for (const auto& entry : entries) {
                if (!entry.IsObject()) {
                    RedLogger::Error(std::format("Category entry {} in {} is malformed: root is not of type object.", i, entryPath));
                    continue;
                }

                if (!entry.HasMember("resourcePath")) {
                    RedLogger::Error(std::format("Category entry {} in {} is malformed: missing property `resourcePath`.", i, entryPath));
                    continue;
                }

                if (!entry["resourcePath"].IsString()) {
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
                    if (entry["appearance"].IsString()) {
                        catEntry.appearance = entry["appearance"].GetString();
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

    std::unordered_map<std::string, VariantPool> InfiniteRandomizerFrameworkNative::LoadVariantPoolsFromDisk() {
        const auto variantPoolDir = std::filesystem::current_path().string() + R"(\plugins\cyber_engine_tweaks\mods\InfiniteRandomizerFramework\data\variantPools)";
        std::unordered_map<std::string, VariantPool> parsedPools;

        std::size_t count = std::distance(
        fs::directory_iterator(variantPoolDir),
        fs::directory_iterator{}
        );

        RedLogger::Debug("Found " + std::to_string(count) + " categories");

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

                if (!entry["resourcePath"].IsString()) {
                    RedLogger::Error(std::format("Variant pool entry {} in {} is malformed: property `resourcePath` is not of type string.", i, entryPath));
                    continue;
                }

                const auto resourcePathString = entry["resourcePath"].GetString();
                RedLogger::Debug(resourcePathString);
                const auto redResourcePath = RED4ext::ResourcePath(resourcePathString);
                RedLogger::Debug(
                    std::to_string(m_depot->ResourceExists(redResourcePath)));
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
                    if (entry["weight"].IsNumber()) {
                        auto weight = entry["weight"].GetFloat();
                        if (weight <= 0.0f) {
                            RedLogger::Warning(std::format("Variant pool entry {} in {} is malformed: property `weight` must be bigger than 0, using default.", i, entryPath));
                            variant.weight = 1.0f;
                        }
                        else {
                            variant.weight = weight;
                        }
                    }
                    else {
                        RedLogger::Warning(std::format("Variant pool entry {} in {} is malformed: property `weight` is not of type number, using default.", i, entryPath));
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
                        RedLogger::Warning(std::format("Variant pool entry {} in {} is malformed: property `appearance` is not of type string, using default.", i, entryPath));
                        variant.appearance = "default";
                    }
                }
                else {
                    variant.appearance = "default";
                }
                RedLogger::Debug(variant.appearance);
                RedLogger::Debug(
                    std::to_string(m_depot->ResourceExists(variant.resourcePath)));
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
