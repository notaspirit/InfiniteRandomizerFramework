---@class Category
---@field name string
---@field resourcePaths table<string>
local category = {}

function category:new(name, resourcePaths)
    local obj = {}

    obj.name = nil
    obj.resourcePaths = {}

    setmetatable(obj, self)
    self.__index = self
    return obj
end

return category