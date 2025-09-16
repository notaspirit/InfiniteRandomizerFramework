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


---@param tableInput table
function deepCopy(tableInput)
    local newTable = {}
    for key, value in pairs(tableInput) do
        if type(value) == "table" then
            newTable[key] = deepCopy(value)
        else
            newTable[key] = value
        end
    end
    return newTable
end

---@param cname CName
function CNameToString(cname)
    return tostring(cname):sub(58, -8)
end

---@param path string
function GetExtension(path)
    local dotPos = path:match(".*()%.")
    return dotPos and path:sub(dotPos + 1) or ""
end

return utils