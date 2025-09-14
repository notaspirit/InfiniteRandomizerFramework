local utils = {}

function utils.isInTable(table, value)
    for _, v in ipairs(table) do
        if v == value then
            return true
        end 
    end 
    return false
end

return utils