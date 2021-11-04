
local function parseArgs(...)
    local paramCount = select('#', ...)
    local txt = ''
    for i = 1, paramCount do
    	local data = tostring(select(i, ...))
        txt = txt .. data

        if i < paramCount then
            txt = txt .. ","
        end
    end
    return txt
end


function logFatal( ... ) luaLog(0, parseArgs(...)) end
function logErr( ... ) luaLog(1, parseArgs(...)) end
function logInfo( ... ) luaLog(2, parseArgs(...)) end
function logDebug( ... ) luaLog(3, parseArgs(...)) end