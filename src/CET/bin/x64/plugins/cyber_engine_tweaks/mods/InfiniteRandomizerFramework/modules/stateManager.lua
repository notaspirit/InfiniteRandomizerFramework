local jsonUtils = require("modules/jsonUtils")
local logger = require("modules/logger")
local utils = require("modules/utils")
local variantPool = require("modules/variantPool")
local variant = require("modules/variant")

local categoriesDir = "data/categories/"
local variantPoolsDir = "data/variantPools/"

local stateManager = {}

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

        local file = io.open(tostring(variantPoolsDir) .. tostring(filePath.name), "r")
        if not file then
            logger.error("Failed to open variant pool file: " .. tostring(filePath.name), true)
            goto continueVPFiles
        end

        local content = file:read("*a")
        file:close()

        local json = jsonUtils.JSONToTable(content)
        if not json then
            logger.error("Failed to parse JSON in variant pool file: " .. tostring(filePath.name), true)
            goto continueVPFiles
        end

        if json.name == nil or json.variants == nil or json.enabled == nil or json.category == nil or #json.variants == 0 then
            local errMsg = "Invalid variant pool format in file: " .. tostring(filePath.name)
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
            v.weight = v.weight or 1
            if v.resourcePath == nil then
                local errMsg = "Invalid variant format in pool: " .. tostring(json.name)
                if (v.resourcePath == nil) then
                    errMsg = errMsg .. " (missing 'resourcePath')"
                end
                logger.warn(errMsg, true)
            else
                if (v.weight < 0) then
                    logger.warn("Skipping variant, weight must be non negative", true)
                    goto continueVariants
                end

                local ext = v.resourcePath:match("([^%.]+)$")
                if (not resourceType) then 
                    resourceType = ext
                end
                if (not (ext == resourceType)) then
                    logger.error("Failed to load variant pool " .. tostring(json.name) .. " found mixed resource types!", true)
                    goto continueVPFiles
                end

                table.insert(variants, variant:new(v.resourcePath, v.appearance, v.weight))
            end
            ::continueVariants::
        end

        local vp = variantPool:new(json.name, json.variants, json.enabled, json.category, resourceType)

        if (#vp.variants == 0) then
            logger.error("Failed to load variant Pool " .. tostring(json.name) .. " variants contains no vaild elements", true)
        end

        if (variantPools[vp.name]) then 
            logger.warn("Duplicate variant pool name found: " .. tostring(vp.name) .. " in file: " .. tostring(filePath.name), true)
        else
            variantPools[vp.name] = vp
            IRF.rawPoolPathLookup[vp.name] = tostring(variantPoolsDir) .. tostring(filePath.name)
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

function stateManager.load()
    loadRawPools()
end

function stateManager.saveRawPool(name)
    local filePath = IRF.rawPoolPathLookup[name]
    if not filePath then
        logger.error("No file path found for variant pool: " .. tostring(name), true)
        return
    end

    local file = io.open(filePath, "w")
    if not file then
        logger.error("Failed to open file for writing: " .. tostring(filePath), true)
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