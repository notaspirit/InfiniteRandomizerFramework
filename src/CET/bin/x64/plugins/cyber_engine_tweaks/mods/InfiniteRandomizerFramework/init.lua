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
    version = "1.1.0",
    targetMeshPaths = {},
    mergedCategories = {},
    rawPools = {},
    rawPoolPathLookup = {},
    categoryTypeLookup = {},
    sortedRawPoolKeys = {},

    poolsToSave = {},

    OverlayOpen = false,
    ReloadFromDiskRequested = false
}

registerForEvent("onInit", function()
    stateManager.load()
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

registerForEvent("onUpdate", function()
    if IRF.OverlayOpen then
        return
    end

    if (#IRF.poolsToSave > 0) then
        print("Saving pool: " .. tostring(IRF.poolsToSave[#IRF.poolsToSave]))
        stateManager.saveRawPool(IRF.poolsToSave[#IRF.poolsToSave])
        table.remove(IRF.poolsToSave)
    end

    if IRF.ReloadFromDiskRequested and #IRF.poolsToSave == 0 then
        print("Reloading from disk...")
        IRF.ReloadFromDiskRequested = false
        InfiniteRandomizerFrameworkNative.LoadFromDisk()
    end
end)

return IRF