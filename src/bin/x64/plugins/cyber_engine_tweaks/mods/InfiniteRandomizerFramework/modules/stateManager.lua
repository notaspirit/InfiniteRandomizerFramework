local jsonUtils = require("modules/jsonUtils")
local logger = require("modules/logger")
local utils = require("modules/utils")
local variantPool = require("modules/variantPool")
local variant = require("modules/variant")

local categoriesDir = "data/categories/"
local variantPoolsDir = "data/variantPools/"

local stateManager = {}

local function loadTargetMeshPaths()

    local success, dirFiles = pcall(function()
        return dir(categoriesDir)
    end)

    if not success then
        logger.error("Failed to get category directory: " .. tostring(dirFiles), true)
        return {}
    end

    local targetMeshPaths = {}
    local catTypeLookup = {}

    for _, filePath in ipairs(dirFiles) do
        if not filePath.name:lower():match("%.json$") then
            goto continueCatFiles
        end

        local file = io.open(categoriesDir .. filePath.name, "r")
        if not file then
            logger.error("Failed to open category file: " .. filePath.name, true)
            goto continueCatFiles
        end

        local content = file:read("*a")
        file:close()

        local json = jsonUtils.JSONToTable(content)
        if not json then
            logger.error("Failed to parse JSON in category file: " .. filePath.name, true)
            goto continueCatFiles
        end

        if json.name == nil or json.resourcePaths == nil or #json.resourcePaths == 0 then
            local errMsg = "Invalid category format in file: " .. filePath.name
            if (json.name == nil) then
                errMsg = errMsg .. " (missing 'name')"
            end
            if (json.resourcePaths == nil) then
                errMsg = errMsg .. " (missing 'resourcePaths')"
            else
                if (#json.resourcePaths == 0) then
                    errMsg = errMsg .. " ('resourcePaths' contains no elements)" 
                end
            end
            
            logger.error(errMsg, true)
            goto continueCatFiles
        end

        local tempTargetMeshPaths = {}

        for k, v in pairs(targetMeshPaths) do
            local tempList = {}
            for _, v in ipairs(v) do 
                table.insert(tempList, v)
            end
            tempTargetMeshPaths[k] = tempList
        end

        local resourceType = nil
        for _, path in ipairs(json.resourcePaths) do
            local ext = path:match("([^%.]+)$")
            if (not resourceType) then
                resourceType = ext
            end

            if (not (ext == resourceType)) then
                logger.error("Failed to load category " .. json.name .. " found mixed resource types!", true)
                goto continueCatFiles
            end

            if (tempTargetMeshPaths[path]) then
                if (not utils.isInTable(tempTargetMeshPaths[path], json.name)) then
                    table.insert(tempTargetMeshPaths[path], json.name)
                end
            else
                tempTargetMeshPaths[path] = { json.name }
            end
        end
        catTypeLookup[json.name] = resourceType
        targetMeshPaths = tempTargetMeshPaths

        ::continueCatFiles::
    end
    IRF.categoryTypeLookup = catTypeLookup
    IRF.targetMeshPaths = targetMeshPaths
end

local function loadRawPools()
    local success, dirFiles = pcall(function()
        return dir(variantPoolsDir)
    end)

    if not success then
        logger.error("Failed to get variant pools directory: " .. tostring(dirFiles), true)
        return {}
    end

    local variantPools = {}
    IRF.rawPoolPathLookup = {}
    for _, filePath in ipairs(dirFiles) do
        if not filePath.name:lower():match("%.json$") then
            goto continueVPFiles
        end

        local file = io.open(variantPoolsDir .. filePath.name, "r")
        if not file then
            logger.error("Failed to open variant pool file: " .. filePath.name, true)
            goto continueVPFiles
        end

        local content = file:read("*a")
        file:close()

        local json = jsonUtils.JSONToTable(content)
        if not json then
            logger.error("Failed to parse JSON in variant pool file: " .. filePath.name, true)
            goto continueVPFiles
        end

        if json.name == nil or json.variants == nil or json.enabled == nil or json.category == nil or #json.variants == 0 then
            local errMsg = "Invalid variant pool format in file: " .. filePath.name
            if (json.name == nil) then
                errMsg = errMsg .. " (missing 'name')"
            end
            if (json.variants == nil) then
                errMsg = errMsg .. " (missing 'variants')"
            else
                if (#json.variants == 0) then
                    errMsg = errMsg .. " ('variants' contains no elements)"
                end
            end
            if (json.enabled == nil) then
                errMsg = errMsg .. " (missing 'enabled')"
            end
            if (json.category == nil) then
                errMsg = errMsg .. " (missing 'category')"
            end
            logger.error(errMsg, true)
            goto continueVPFiles
        end

        local variants = {}
        local resourceType = nil
        for _, v in ipairs(json.variants) do
            if v.resourcePath == nil or v.weight == nil or v.appearance == nil then
                local errMsg = "Invalid variant format in pool: " .. json.name
                if (v.resourcePath == nil) then
                    errMsg = errMsg .. " (missing 'resourcePath')"
                end
                if (v.weight == nil) then
                    errMsg = errMsg .. " (missing 'weight')"
                end
                if (v.appearance == nil) then
                    errMsg = errMsg .. " (missing 'appearance')"
                end
                logger.warn(errMsg, true)
            else
                local ext = v.resourcePath:match("([^%.]+)$")
                if (not resourceType) then 
                    resourceType = ext
                end
                if (not (ext == resourceType)) then
                    logger.error("Failed to load variant pool " .. json.name .. " found mixed resource types!", true)
                    goto continueVPFiles
                end

                table.insert(variants, variant:new(v.resourcePath, v.appearance, v.weight))
            end
        end

        local vp = variantPool:new(json.name, json.variants, json.enabled, json.category, resourceType)

        if (#vp.variants == 0) then
            logger.error("Failed to load variant Pool " .. json.name .. " variants contains no vaild elements", true)
        end

        if (variantPools[vp.name]) then 
            logger.warn("Duplicate variant pool name found: " .. vp.name .. " in file: " .. filePath.name, true)
        else
            variantPools[vp.name] = vp
            IRF.rawPoolPathLookup[vp.name] = variantPoolsDir .. filePath.name
        end

        ::continueVPFiles::
    end

    IRF.sortedRawPoolKeys = {}

    for k, _ in pairs(variantPools) do
        table.insert(IRF.sortedRawPoolKeys, k)
    end

    table.sort(IRF.sortedRawPoolKeys)

    IRF.rawPools = variantPools
end

local function buildMergedCategories()
    local mergedPools = {}
    for _, vp in pairs(IRF.rawPools) do
        if not vp.enabled then
            goto continueMergePools
        end

        if not (vp.resourceType == IRF.categoryTypeLookup[vp.category]) then
            logger.error("Failed to add variant Pool " .. vp.name .. " to category " .. vp.category .. " due to resource type mismatch. Pool is of type " .. vp.resourceType .. " category is " .. IRF.categoryTypeLookup[vp.category], true)
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

    IRF.mergedCategories = mergedPools
end

function stateManager.load()
    loadTargetMeshPaths()
    loadRawPools()
    buildMergedCategories()
end

function stateManager.refreshEnabledPools()
    buildMergedCategories()
end

function stateManager.saveRawPool(name)
    local filePath = IRF.rawPoolPathLookup[name]
    if not filePath then
        logger.error("No file path found for variant pool: " .. name, true)
        return
    end

    local file = io.open(filePath, "w")
    if not file then
        logger.error("Failed to open file for writing: " .. filePath, true)
        return
    end
    local resourceType = IRF.rawPools[name].resourceType
    IRF.rawPools[name].resourceType = nil
    local content = jsonUtils.TableToJSON(IRF.rawPools[name])
    IRF.rawPools[name].resourceType = resourceType
    file:write(content)
    file:close()
end

return stateManager