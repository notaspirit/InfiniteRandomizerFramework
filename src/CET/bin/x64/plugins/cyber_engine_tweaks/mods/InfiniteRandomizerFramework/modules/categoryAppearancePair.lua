---@class CategoryAppearancePair
---@field appearance string
---@field cateogory string
local categoryAppearancePair = {}

function categoryAppearancePair:new(app, category)
    local obj = {}

    obj.category = category
    obj.appearance = app or nil

    setmetatable(obj, self)
    self.__index = self
    return obj
end

return categoryAppearancePair