local stateManager = require("modules/stateManager")
local jsonUtils = require("modules/jsonUtils")

local gui = {}

function gui.draw() 
    if ImGui.Begin("Infinite Randomizer Framework") then
        if ImGui.Button("Refresh Pool State") then
            stateManager.refreshEnabledPools()
        end
        if ImGui.Button("Reload From Disk") then
            stateManager.load()
        end
        if ImGui.Button("Print State to Console") then
            print(jsonUtils.TableToJSON(IRF.targetMeshPaths))
            print(jsonUtils.TableToJSON(IRF.mergedCategories))
            print(jsonUtils.TableToJSON(IRF.rawPools))
        end

        -- TODO 
        -- List all pools and their enabled/disabled state, allow toggling and save to disk
        ImGui.End()
    end
end

return gui