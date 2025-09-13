---@class IRFVariant
---@field resourcePath string
---@field weight number
local variant = {}

function variant:new()
    local obj = {}

    obj.resourcePath = nil
    obj.weight = 1

    setmetatable(obj, self)
    self.__index = self
    return obj
end

return variant