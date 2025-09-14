local utils = {}

function utils.isInTable(table, value)
    for _, v in ipairs(table) do
        if v == value then
            return true
        end 
    end 
    return false
end

function switch(value, cases)
    local matched = false
    for i, case in ipairs(cases) do
        if case.value == value or matched then
            matched = true
            case.action()
            if case.break_ then
                break
            end
        end
    end
end

return utils