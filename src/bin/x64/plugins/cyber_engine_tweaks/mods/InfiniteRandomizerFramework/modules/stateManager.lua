local jsonUtils = require("modules/jsonUtils")
local category = require("modules/category")
local utils = require("modules/utils")
local variantPool = require("modules/variantPool")
local variant = require("modules/variant")

local categoriesDir = "data/categories/"
local variantPoolsDir = "data/variantPools/"

local stateManager = {}

function stateManager.load()
    IRF.targetMeshPaths = loadTargetMeshPaths()
    IRF.rawPools = loadRawPools()
    IRF.mergedCategories = buildMergedCategories()
end

function stateManager.refreshEnabledPools()
    IRF.mergedCategories = buildMergedCategories()
end

function loadTargetMeshPaths()

    local success, dirFiles = pcall(function()
        return dir(categoriesDir)
    end)

    if not success then
        print("Error loading categories directory: " .. tostring(dirFiles))
        return {}
    end

    local targetMeshPaths = {}

    for _, filePath in ipairs(dirFiles) do
        if not filePath.name:lower():match("%.json$") then
            goto continueCatFiles
        end

        local file = io.open(categoriesDir .. filePath.name, "r")
        if not file then
            print("Error opening category file: " .. filePath.name)
            goto continueCatFiles
        end

        local content = file:read("*a")
        file:close()

        local json = jsonUtils.JSONToTable(content)
        if not json then
            print("Error parsing JSON in category file: " .. filePath.name)
            goto continueCatFiles
        end

        print(jsonUtils.TableToJSON(json))

        if not json.name or not json.resourcePaths then
            print("Invalid category format in file: " .. filePath.name)
            goto continueCatFiles
        end

        for _, path in ipairs(json.resourcePaths) do
            if (targetMeshPaths[path]) then
                if (not utils.isInTable(targetMeshPaths[path], json.name)) then
                    table.insert(targetMeshPaths[path], json.name)
                end
            else
                targetMeshPaths[path] = { json.name }
            end
        end

        ::continueCatFiles::
    end

    return targetMeshPaths
end

function loadRawPools()
    local success, dirFiles = pcall(function()
        return dir(variantPoolsDir)
    end)

    if not success then
        print("Error loading variant pools directory: " .. tostring(dirFiles))
        return {}
    end

    local variantPools = {}

    for _, filePath in ipairs(dirFiles) do
        if not filePath.name:lower():match("%.json$") then
            goto continueVPFiles
        end

        local file = io.open(variantPoolsDir .. filePath.name, "r")
        if not file then
            print("Error opening variant pool file: " .. filePath.name)
            goto continueVPFiles
        end

        local content = file:read("*a")
        file:close()

        local json = jsonUtils.JSONToTable(content)
        if not json then
            print("Error parsing JSON in variant pool file: " .. filePath.name)
            goto continueVPFiles
        end

        if not json.name or not json.variants or not json.enabled or not json.category then
            print("Invalid variant pool format in file: " .. filePath.name)
            goto continueVPFiles
        end

        local variants = {}
        for _, v in ipairs(json.variants) do
            if not v.resourcePath or not v.weight or not v.appearance then
                print("Invalid variant format in pool file: " .. filePath.name)
            else
                table.insert(variants, variant:new(v.resourcePath, v.appearance, v.weight))
            end
        end

        local vp = variantPool:new(json.name, json.variants, json.enabled, json.category)

        if (variantPools[vp.name]) then
            print("Duplicate variant pool name found: " .. vp.name .. " in file: " .. filePath.name)
        else
            variantPools[vp.name] = vp
        end

        ::continueVPFiles::
    end

    return variantPools
end

function buildMergedCategories()
    local mergedPools = {}

    print("Building merged categories from raw pools...")

    for _, vp in pairs(IRF.rawPools) do
        print("Processing pool: " .. vp.name .. " (Enabled: " .. tostring(vp.enabled) .. ", Category: " .. tostring(vp.category) .. ")")
        if not vp.enabled then
            goto continueMergePools
        end

        if (not mergedPools[vp.category]) then
            mergedPools[vp.category] = {}
        end

        for _, variant in pairs(vp.variants) do
            if (not utils.isInTable(mergedPools[vp.category], variant)) then
                table.insert(mergedPools[vp.category], variant)
            end
        end
        ::continueMergePools::
    end

    return mergedPools
end

return stateManager