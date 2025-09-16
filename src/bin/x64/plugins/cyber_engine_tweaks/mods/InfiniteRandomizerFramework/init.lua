--- Infinite Randomizer Framework (IRF) for Cyber Engine Tweaks
--- Version 1.0.0
--- Author: sprt_

local stateManager = require("modules/stateManager")
local logger = require("modules/logger")
local gui = require("modules/gui")
local utils = require("modules/utils")

---@class IRF
---@field version string
---@field targetMeshPaths table<string, table<string, table<string>>>
---@field rawPools table<VariantPool>
---@field mergedCategories table<string, table<IRFVariant>>
---@field rawPoolPathLookup table<string, string>
---@field categoryTypeLookup table<string, string>
---@field sortedRawPoolKeys table<string>
---@field OverlayOpen boolean
IRF = {
    version = "1.0.0",
    targetMeshPaths = {},
    mergedCategories = {},
    rawPools = {},
    rawPoolPathLookup = {},
    categoryTypeLookup = {},
    sortedRawPoolKeys = {},
    OverlayOpen = false
}

local function getSimpleNodeType(node)
    if (node:IsA("worldMeshNode") or
        node:IsA("worldInstancedMeshNode") or
        node:IsA("worldBendedMeshNode") or
        node:IsA("worldFoliageNode")) then
        return "mesh"
    end

    if (node:IsA("worldTerrainMeshNode")) then
        return "terrainMesh"
    end

    if (node:IsA("worldEntityNode")) then
        return "entity"
    end

    if (node:IsA("worldStaticDecalNode")) then
        return "decal"
    end

    return "invalid"
end

local function getAppName(node, st)
    if (st == "mesh") then
        return CNameToString(node.meshAppearance)
    end
    if (st == "entity") then
        return CNameToString(node.appearanceName)
    end
end

local function OnSectorLoad(class, event)
    local resource = event:GetResource()
    if (not IsDefined(resource)) then return end
    if (not resource:IsA('worldStreamingSector')) then return end

    local nodeCount = resource:GetNodeCount()
    for i = 0, nodeCount - 1 do
        local node = resource:GetNode(i)
        if (not IsDefined(node)) then goto continueNodes end
        local sType = getSimpleNodeType(node)
        if (sType == "invalid") then
            goto continueNodes
        end

        local resPath = nil
        if (sType == "mesh") then
            resPath = ResRef.FromHash(node.mesh.hash):ToString()
        elseif (sType == "terrainMesh") then
            resPath = ResRef.FromHash(node.meshRef.hash):ToString()
        elseif (sType == "entity") then
            resPath = ResRef.FromHash(node.entityTemplate.hash):ToString()
        elseif (sType == "decal") then
            resPath = ResRef.FromHash(node.material.hash):ToString()
        else
            logger.warn("Couldn't determine resource path, skipping", true)
            goto continueNodes
        end

        local meshEntry = IRF.targetMeshPaths[resPath]
        if (not meshEntry) then goto continueNodes end

        local app = getAppName(node, sType)
        local mergedCats = {}
        local totalWeight = 0
        for _, catApp in ipairs(meshEntry[app] or {}) do
            if not IRF.mergedCategories[catApp] then goto continue_catApp end
            for _, v in ipairs(IRF.mergedCategories[catApp]) do
                if utils.isInTable(mergedCats, v) then goto continue_v end
                table.insert(mergedCats, v)
                totalWeight = totalWeight + v.weight
                ::continue_v::
            end
            ::continue_catApp::
        end

        for _, catApp in ipairs(meshEntry[AnyApp] or {}) do
            if not IRF.mergedCategories[catApp] then goto continue_catApp1 end
            for _, v in ipairs(IRF.mergedCategories[catApp]) do
                if utils.isInTable(mergedCats, v) then goto continue_v1 end
                table.insert(mergedCats, v)
                totalWeight = totalWeight + v.weight
                ::continue_v1::
            end
            ::continue_catApp1::
        end

        if totalWeight == 0 then goto continueNodes end
        local randomValue = math.random(1, totalWeight)
        local selectedVariant = nil
        for _, variant in ipairs(mergedCats) do
            randomValue = randomValue - variant.weight
            if randomValue <= 0 then
                selectedVariant = variant
                break
            end
        end
        if not selectedVariant then goto continueNodes end

        if not Game.GetResourceDepot().ResourceExists(ResRef.FromString(selectedVariant.resourcePath)) then
            logger.warn("Skipping non-existent resource: " .. tostring(selectedVariant.resourcePath), true)
            goto continueNodes
        end
        local existingExt = GetExtension(resPath)
        local replacementExt = GetExtension(selectedVariant.resourcePath)
        if existingExt ~= replacementExt then
            logger.warn("Skipping mismatched resource: " .. tostring(selectedVariant.resourcePath) .. " (expected " .. tostring(existingExt) .. " got " .. tostring(replacementExt) .. ")", true)
            goto continueNodes
        end

        if sType == "mesh" then
            node.mesh = selectedVariant.resourcePath
            node.meshAppearance = selectedVariant.appearance
        elseif sType == "terrainMesh" then
            node.meshRef = selectedVariant.resourcePath
        elseif sType == "entity" then
            node.entityTemplate = selectedVariant.resourcePath
            node.appearanceName = selectedVariant.appearance
            node.instanceData = entEntityInstanceData:new()
        elseif sType == "decal" then
            node.material = selectedVariant.resourcePath
        end
        ::continueNodes::
    end
end

registerForEvent("onInit", function()
    stateManager.load()
    ObserveBefore("InfiniteRandomizerFramework", "OnSectorLoad", OnSectorLoad)
    logger.info("Infinite Randomizer Framework (IRF) v" .. tostring(IRF.version) .. " initialized.", true)
end)

registerForEvent("onOverlayOpen", function()
    IRF.OverlayOpen = true
end)

registerForEvent("onOverlayClose", function()
    IRF.OverlayOpen = false
end)

registerForEvent("onDraw", function()
    if not IRF.OverlayOpen then return end
    gui.draw()
end)

return IRF