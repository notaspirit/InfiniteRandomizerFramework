---@class TargetMeshPath
---@field appearance string
---@field cateogories table<string>
local categoryEntry = {}

function categoryEntry:new(app, categories)
    local obj = {}

    obj.categories = categories or {}
    obj.appearance = app or nil

    setmetatable(obj, self)
    self.__index = self
    return obj
end

return categoryEntry