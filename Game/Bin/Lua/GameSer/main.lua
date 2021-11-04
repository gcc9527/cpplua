
require("Lua.Common.log")

gcfg = {}

_G.myRequire = function(path, flag)
    require(path)
    logInfo(path)
    if flag ~= true then
        gCacheModule[path] = true
    end
end


-- 热更新不会生效的文件
_G.myRequire("Lua.Common.debug", true)
_G.myRequire("Lua.GameSer.global", true)










-- 热更新会生效的文件
_G.myRequire("Lua.GameSer.Config")
_G.myRequire("Lua.GameSer.event")
_G.myRequire("Lua.Common.tools")
_G.myRequire("Lua.Common.msgdef")
_G.myRequire("Lua.Common.parseProto")
_G.myRequire("Lua.Common.gm")
_G.myRequire("Lua.GameSer.MsgHandle.testHandle")
_G.myRequire("Lua.GameSer.Manager.testMgr")










