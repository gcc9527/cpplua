
if ENUM_PLATFORM_WIN then
    package.cpath = package.cpath .. ";../Libs/?.dll"
end

if ENUM_PLATFORM_LINUX then
    package.cpath = package.cpath .. ";../Libs/?.so"
end


require("Lua.Common.luaPanda").start("127.0.0.1", 8818)
