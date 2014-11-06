function map(fn, arr)
    local new_arr = {}
    for i,v in ipairs(arr) do
        new_arr[i] = fn(v)
    end
    return new_arr
end
