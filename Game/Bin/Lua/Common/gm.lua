
gmMgr = {}

function gmMgr.rs(path)
    if path == nil then
        for k, v in pairs(gCacheModule) do
            if package.loaded[k] then
                package.loaded[k] = nil
                gCacheModule[k] = nil
            end
        end

        require("Lua.GameSer.main")
        if package.loaded["Lua.GameSer.main"] then
            package.loaded["Lua.GameSer.main"] = nil
        end
    else
        if gCacheModule[path] and package.loaded[path] then
            gCacheModule[path] = nil
            package.loaded[path] = nil
            package.myRequire(path)
        end
    end
end