require('util')

function run_repl()
    repeat
        local s = readline('> ')
        if s == 'exit' then break end

        local f, err = load('_last_repl_result = {' .. s .. '}', 'stdin')
        if err then
            f = load(s, 'stdin')
        end

        if f then
            _last_repl_result = nil
            pcall(f)
            res = _last_repl_result
            if res and #res > 0 then
                writeline(table.concat(map(tostring, res), '; '))
            end
        else
            writeline(err)
        end
    until false
end
