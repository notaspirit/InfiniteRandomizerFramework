--- Infinite Randomizer Framework (IRF) for Cyber Engine Tweaks
--- Version 1.1.0
--- Author: sprt_

local stateManager = require("modules/stateManager")
local logger = require("modules/logger")
local gui = require("modules/gui")
local utils = require("modules/utils")

---@class IRF
---@field version string
---@field rawPools table<VariantPool>
---@field rawPoolPathLookup table<string, string>
---@field sortedRawPoolKeys table<string>
---@field OverlayOpen boolean
IRF = {
    version = "1.1.0",
    rawPools = {},
    rawPoolPathLookup = {},
    sortedRawPoolKeys = {},

    OverlayOpen = false
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

return IRF