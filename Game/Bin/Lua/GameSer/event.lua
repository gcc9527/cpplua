

function gServerUpdate(curTime)
    tools.luaGc(curTime)
end

function gServerCmd(cmd)
    local args = tools.split(cmd)
    tools.pt(args)
    local args1 = args[1]

    local s, _ = string.find(args1, "@")
    if s ~= nil then

    else
        local func = gmMgr[args1]
        if type(func) == "function" then
            func(args[2])
        end
    end
end

