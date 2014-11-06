local function main()
    print(package.path)
    require('repl')
    run_repl()
end

status, err = pcall(main)
if status then
    -- no errors
else
    print(err)
end
