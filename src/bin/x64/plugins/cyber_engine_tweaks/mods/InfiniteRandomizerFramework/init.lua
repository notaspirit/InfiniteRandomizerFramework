--- Infinite Randomizer Framework (IRF) for Cyber Engine Tweaks
--- Version 1.0.0
--- Author: sprt_

local stateManager = require("modules/stateManager")
local logger = require("modules/logger")
local gui = require("modules/gui")

---@class IRF
---@field version string
---@field targetMeshPaths table<string, table<CategoryAppearancePair>>
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
        switch(sType, {
            { value = "mesh", action = function() resPath = ResRef.FromHash(node.mesh.hash):ToString() end, break_ = true },
            { value = "terrainMesh", action = function() resPath = ResRef.FromHash(node.meshRef.hash):ToString() end, break_ = true },
            { value = "entity", action = function() resPath = ResRef.FromHash(node.entityTemplate.hash):ToString() end, break_ = true },
            { value = "decal", action = function() resPath = ResRef.FromHash(node.material.hash):ToString() end, break_ = true  }
        })

        if (resPath == nil) then
            logger.warn("Couldn't determine resource path, skipping", true)
            goto continueNodes
        end
        local meshEntry = IRF.targetMeshPaths[resPath]
        if (not meshEntry) then goto continueNodes end

        local cat = {}
        local addedCatNames = {}
        local app = nil
        for _, cname in ipairs(meshEntry) do
            if (not (cname.appearance == nil)) then 
                local appMatch = nil
                if (app == nil) then
                    app = getAppName(node, sType)
                end
                switch(sType, {
                    { value = "mesh", action = function() appMatch = (cname.appearance == app) end, break_ = true },
                    { value = "terrainMesh", action = function() appMatch = true end, break_ = true },
                    { value = "entity", action = function() appMatch = (cname.appearance == app) end, break_ = true },
                    { value = "decal", action = function() appMatch = true end, break_ = true  }
                    -- decal and terrainMesh have no appearance so the app gets ignored
                })
                if not appMatch then
                    goto continueCat
                end
            end
            if (addedCatNames[cname.category]) then
                goto continueCat
            end
            addedCatNames[cname.category] = true

            local catVariants = IRF.mergedCategories[cname.category]
            if (catVariants) then
                for _, variant in ipairs(catVariants) do
                    table.insert(cat, variant)
                end
            end
            ::continueCat::
        end

        local maxWeight = 0
        for _, variant in ipairs(cat) do
            maxWeight = maxWeight + variant.weight
        end
        if (maxWeight == 0) then goto continueNodes end
        local randomValue = math.random(1, maxWeight)

        local randomIndex = 1
        for j, variant in ipairs(cat) do
            randomValue = randomValue - variant.weight
            if (randomValue <= 0) then
                randomIndex = j
                break
            end
        end

        if (not Game.GetResourceDepot().ResourceExists(ResRef.FromString(cat[randomIndex].resourcePath))) then
            logger.warn("Skipping non-existent resource: " .. tostring(cat[randomIndex].resourcePath), true)
            goto continueNodes
        end

        local existingExt = GetExtension(resPath)
        local replacementExt = GetExtension(cat[randomIndex].resourcePath)
        if (not (existingExt == replacementExt)) then
            logger.warn("Skipping mismatched resource: " .. tostring(cat[randomIndex].resourcePath) .. " (expected " .. tostring(existingExt) .. " got " .. tostring(replacementExt) .. ")", true)
            goto continueNodes
        end

        switch(sType, {
            { value = "mesh", action = function() node.mesh = cat[randomIndex].resourcePath; node.meshAppearance = cat[randomIndex].appearance end, break_ = true },
            { value = "terrainMesh", action = function() node.meshRef = cat[randomIndex].resourcePath end, break_ = true },
            { value = "entity", action = function() node.entityTemplate = cat[randomIndex].resourcePath; node.appearanceName = cat[randomIndex].appearance; node.instanceData = entEntityInstanceData:new()  end, break_ = true },
            { value = "decal", action = function() node.material = cat[randomIndex].resourcePath end, break_ = true  }
        })
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