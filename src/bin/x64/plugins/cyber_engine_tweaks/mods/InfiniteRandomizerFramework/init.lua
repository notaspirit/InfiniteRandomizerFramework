--- Infinite Randomizer Framework (IRF) for Cyber Engine Tweaks
--- Version 1.0.0
--- Author: sprt_

local stateManager = require("modules/stateManager")
local jsonUtils = require("modules/jsonUtils")
local gui = require("modules/gui")

---@class IRF
---@field version string
---@field targetMeshPaths table<string, table<string>>
---@field rawPools table<VariantPool>
---@field mergedCategories table<string, table<IRFVariant>>
---@field OverlayOpen boolean
IRF = {
    version = "1.0.0",
    targetMeshPaths = {},
    mergedCategories = {},
    rawPools = {},
    OverlayOpen = false
}

local inputListener
registerForEvent('onHook', function()
    inputListener = NewProxy({
        OnSectorLoaded = {
            args = {"handle:ResourceEvent"},
            callback = function (event)
                local resource = event:GetResource()
                if (not IsDefined(resource)) then return end
                if (not resource:IsA('worldStreamingSector')) then return end

                local nodeCount = resource:GetNodeCount()
                for i = 0, nodeCount - 1 do
                    local node = resource:GetNode(i)
                    if (node == nil) then goto continueNodes end
                    if (node.mesh == nil) then goto continueNodes end
                    if (not node:IsA("worldMeshNode") and
                     not node:IsA("worldInstancedMeshNode") and
                     not node:IsA("worldBendedMeshNode") and
                     not node:IsA("worldTerrainMeshNode") and
                     not node:IsA("worldFoliageNode"))
                     then goto continueNodes end

                    local resPath = ResRef.FromHash(node.mesh.hash):ToString()
                    local catName = IRF.targetMeshPaths[resPath]
                    if (not catName) then goto continueNodes end
                    -- TODO handle multiple categories per mesh (merge pools)
                    local cat = IRF.mergedCategories[catName[1]]

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
                
                    if (not ResRef.FromString(cat[randomIndex].resourcePath):IsValid()) then
                        print("[WARNING] Skipping invalid resource path: " .. tostring(cat[randomIndex].resourcePath))
                        goto continueNodes
                    end

                    -- add check to make sure the mesh path exists and the resource is valid to avoid crashes, if it isn't regamble with an attempt limit to avoid infinite loops
                    print("Random Index: " .. tostring(randomIndex))
                    print(cat[randomIndex].resourcePath)
                    print(cat[randomIndex].appearance)
                    -- print("Old " .. ResRef.FromHash(node.mesh.hash):ToString())
                    node.mesh = cat[randomIndex].resourcePath
                    node.meshAppearance = cat[randomIndex].appearance
                    
                    -- print("New " .. ResRef.FromHash(node.mesh.hash):ToString())
                    ::continueNodes::
                end
            end
        }
    })
    Game.GetCallbackSystem():RegisterCallback('Resource/PostLoad', inputListener:Target(), inputListener:Function('OnSectorLoaded'), true):AddTarget(ResourceTarget.Type("worldStreamingSector"))
end)

registerForEvent("onInit", function()
    stateManager.load()
    print("Infinite Randomizer Framework (IRF) v" .. IRF.version .. " initialized.")
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