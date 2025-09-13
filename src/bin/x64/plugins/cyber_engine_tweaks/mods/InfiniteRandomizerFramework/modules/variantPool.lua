
---@class VariantPool
---@field name string
---@field variants table<IRFVariant>
---@field enabled boolean
---@field category string
local variantPool = {}

function variantPool:new(name, variants, enabled, category)
    local obj = {}

    obj.name = name
    obj.variants = variants or {}
    obj.enabled = (enabled == nil) and true or enabled
    obj.category = category

    setmetatable(obj, self)
    self.__index = self
    return obj
end

return variantPool