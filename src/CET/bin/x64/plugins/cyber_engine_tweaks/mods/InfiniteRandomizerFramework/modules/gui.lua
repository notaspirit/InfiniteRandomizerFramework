local stateManager = require("modules/stateManager")
local logger = require("modules/logger")
local jsonUtils = require("modules/jsonUtils")

local gui = {}

function gui.draw() 
    if ImGui.Begin("Infinite Randomizer Framework") then
        if ImGui.Button("Reload From Disk") then
            stateManager.load()
        end
        ImGui.Separator()

        if (ImGui.BeginTable("Variant Pools", 4,  ImGuiTableFlags.SizingFixedFit)) then
            ImGui.TableSetupColumn("Enabled")
            ImGui.TableSetupColumn("Pool Name")
            ImGui.TableSetupColumn("Category")
            ImGui.TableSetupColumn("Variants")
            ImGui.TableHeadersRow()

            for _, k in ipairs(IRF.sortedRawPoolKeys) do
                local poolObj = IRF.rawPools[k]
                ImGui.TableNextRow()
                ImGui.TableSetColumnIndex(0)
                poolObj.enabled, changed = ImGui.Checkbox("##" .. tostring(poolObj.name), poolObj.enabled)
                if changed then
                    table.insert(IRF.poolsToSave, poolObj.name)
                    IRF.ReloadFromDiskRequested = true
                    print("Marked " .. tostring(poolObj.name) .. " for saving.")
                end

                ImGui.TableSetColumnIndex(1)
                ImGui.Text(poolObj.name)
                ImGui.TableSetColumnIndex(2)
                ImGui.Text(poolObj.category)
                ImGui.TableSetColumnIndex(3)
                ImGui.Text(tostring(#poolObj.variants))
            end

            ImGui.EndTable()
        end
        ImGui.End()
    end
end

return gui