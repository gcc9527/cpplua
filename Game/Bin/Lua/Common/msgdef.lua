

local cacheMsg = {}

local function regMsg(msgId, msgName, lang, desc)
    local name = cacheMsg[msgId]
    if name ~= nil or name == msgName then
        logErr("exist msgid = , or msgName repeated", msgId)
        return
    end

    cacheMsg[msgId] = msgName
    gMainThread:cacheMessage(msgId, lang, msgName, desc)
end


regMsg(100,                     "Person",                                                               "cpp",     "Person")
regMsg(101,                     "Person",                                                               "lua",     "Person")
regMsg(102,                     "msgPlayerLogionReq",                                                   "lua",     "玩家登入")
regMsg(103,                     "msgPlayerLogionReq3",                                                  "lua",     "玩家登入")

