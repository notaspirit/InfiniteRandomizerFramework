---@class IRFVariant
---@field resourcePath string
---@field weight number
local variant = {}

function variant:new(resourcePath, appearance, weight)
    local obj = {}

    obj.resourcePath = resourcePath
    obj.appearance = appearance or "default"
    obj.weight = weight or 1

    setmetatable(obj, self)
    self.__index = self
    return obj
end

return variant