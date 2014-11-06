require('utils.lua')

repeat -- REPL
    local s = readline('> ')
    if s == 'exit' then break end

    local f, err = load('_last_repl_result = {' .. s .. '}', 'stdin')
    if err then
        f = load(s, 'stdin')
    end

    if f then
        _last_repl_result = nil
        pcall(f)
        if _last_repl_result and #_last_repl_result > 0 then
            writeline(table.concat(map(tostring, _last_repl_result), '; '))
        end
    else
        writeline(err)
    end
until false
