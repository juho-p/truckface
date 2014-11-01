-- source: http://stackoverflow.com/questions/20410082/why-does-the-lua-repl-require-you-to-prepend-an-equal-sign-in-order-to-get-a-val

local function print_results(...)
    -- This function takes care of nils at the end of results and such.
    if select('#', ...) > 1 then
        print(select(2, ...))
    end
end

repeat -- REPL
    io.write'> '
    local s = io.read()
    if s == 'exit' then break end

    local f, err = load(s, 'stdin')
    if err then -- Maybe it's an expression.
        -- This is a bad hack, but it might work well enough.
        f = load('return (' .. s .. ')', 'stdin')
    end

    if f then
        print_results(pcall(f))
    else
        print(err)
    end
until false
