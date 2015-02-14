-- map array arr with function fn
function map(fn, arr)
    local new_arr = {}
    for i,v in ipairs(arr) do
        new_arr[i] = fn(v)
    end
    return new_arr
end

-- filter array arr using function fn
function filter(fn, arr)
    local new_arr = {}
    j = 1
    for i,v in ipairs(arr) do
        if fn(v) then
            new_arr[j] = v
            j = j + 1
        end
    end
    return new_arr
end

function reduce(fn, arr, start)
    result = start
    for i,v in ipairs(arr) do
        result = fn(result, v)
    end
    return result
end
