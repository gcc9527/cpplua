

local Table_AchievementCondition = require("Config.Table_AchievementCondition")
local BaseUtil = require("Util.BaseUtil")

EventCollectionMgr = {}
EventCollectionMgr.Reg = {}
--[[
EventCollectionMgr.Reg =
{
    [模块id] =
    {
        [事件类型] =
        {
            [任务id] = 事件唯一id,
        },
    },
}
--]]

-- 事件类型定义
EventCollectionMgr.EVENT_COLLECTION_TYPE =
{
    PASS_APPOINT_STAGE = 1, -- 通关指定副本
    KILL_MONSTER = 2, -- 击杀指定id的怪物
    GET_APPOINT_ITEM = 3, -- 获得指定道具
    DEDUCT_APPOINT_ITEM = 4, -- 扣除指定道具
    GET_APPOINT_LEVEL_HERO = 5, -- 获得指定等级的英雄
    PLAYER_ACHIEVE_APPOINT_LEVEL = 6, -- 玩家到达指定等级
    GET_APPOINT_STAR_HERO = 7, -- 获得指定星级的英雄
    COLLECTION_BOOKS = 8, -- 收集图鉴
    HERO_INTIMACY_ACHIEVE_VAL = 9,  -- 英雄亲密度达到指定值
    HERO_SKILL_LEVEL_ACHIEVE_VAL = 10, -- 英雄升级技能到达指定等级
    ACTIVE_HERO_TALEN = 11, -- 激活英雄天赋
    HERO_LOTTERY_ACHIEVE_VAL = 12, -- 英雄召唤达到指定次数
    PASS_URGENT_TASK = 13, -- 完成紧急任务
    PASS_GUILD_TASK = 14, -- 完成公会任务
    PASS_CHAPTER_TASK = 15, -- 完成章节任务
    HERO_TALEN_UP_LEVEL = 16, -- 英雄天赋升级
    PASS_SPECIAL_TYPE_STAGE = 17, -- 通关特定类型副本
    CHALLENGE_ARENA = 18, -- 挑战竞技场
    ARENA__SCORE_ACHIEVE_VAL = 19, -- 竞技场积分达到指定值
    CUMULATIVE_LOGION_DAYS = 20, -- 累计登入达到指定天数
}

-- 事件值更新类型
EventCollectionMgr.EVENT_COLLECTION_VAL_TYPE =
{
    ADDUP = 0, -- 累计增加
    REPUP = 1, -- 替换掉
}

-- 需要注册事件的模块
EventCollectionMgr.EVENT_REG_MODULE =
{
    [1] = 0, -- 成就模块
    [2] = 0, -- 战令模块
    [3] = 0, -- 冒险家之路模块
}

EventCollectionMgr.RESETTYPE = -- 重置类型
{
    days = 0, -- 每天
    week = 1, -- 每周
    forever = 2 -- 永久
}




function EventCollectionMgr.getModuleData(moduleId, player)
    local func = EventCollectionMgr.EVENT_REG_MODULE[moduleId]
    if type(func) == "function" then
        return func(player)
    end

    return nil
end

function EventCollectionMgr.regDataFunc(moduleId, func)
    if type(func) ~= "function" then
        BaseUtil.PrintEx("EventCollectionMgr.regDataFunc err, no function type = ", func)
        return
    end

    if EventCollectionMgr.EVENT_REG_MODULE[moduleId] == nil then
        BaseUtil.PrintEx("EventCollectionMgr.regDataFunc err, no moduleId = ", moduleId)
        return
    end
    EventCollectionMgr.EVENT_REG_MODULE[moduleId] = func
end


function EventCollectionMgr.checkEventExist(eventType)
    for k, v in pairs(EventCollectionMgr.EVENT_COLLECTION_TYPE) do
        if eventType == v then
            return true
        end
    end

    return false
end

function EventCollectionMgr.regEventModule()
    for k, v in pairs(EventCollectionMgr.EVENT_REG_MODULE) do
        if EventCollectionMgr.Reg[k] == nil then
            EventCollectionMgr.Reg[k] = {}
        end
    end
end


-- eventId Table_AchievementCondition中的唯一Id
-- taskId 各自模块中的配置Id(如:成就就是成就id, 任务就是任务id, 其他类似)
function EventCollectionMgr.regEvent(eventId, taskId, eventType, moduleId)
    for k, v in pairs(EventCollectionMgr.Reg) do
        if EventCollectionMgr.checkEventExist(eventType) == false then
            BaseUtil.PrintEx("EventCollectionMgr.regEvent err, no support eventId = , taskId = , eventType = ,", eventId, taskId, eventType)
            return
        end
    end

    local data = EventCollectionMgr.Reg[moduleId]
    if data == nil then
        return
    end


    local t = data[eventType]
    if t == nil then
        data[eventType] = {}
        t = data[eventType]
    end

    t[taskId] = eventId
end




-- 外部模块统一事件调用接口
function EventCollectionMgr.onEvent(player, eventType, eventValType, param1, param2, param3)
    if eventType == nil or eventType <= 0 or eventValType == nil or param1 == nil or param2 == nil or param3 == nil then
        BaseUtil.PrintEx("EventCollectionMgr.onEvent err", eventType, eventValType, param1, param2, param3)
        return
    end

    for k, v in pairs(EventCollectionMgr.Reg) do
        local moduleDatas = EventCollectionMgr.getModuleData(k, player)
        local updateList = {}
        if type(moduleDatas) == "table" then
            local data = v[eventType]
            if data ~= nil then
                updateList = EventCollectionMgr.update(player, moduleDatas, data, eventType, eventValType, param1, param2, param3)
            end
        end

        if not BaseUtil.IsTableEmpty(updateList) then
            if k == 1 then
                AchievementMgr.notiflyClient(player, updateList)
            elseif k == 2 then
                WarorderMgr.notiflyClient(player, updateList)
            elseif k == 3 then
                AdventurerRoadMgr.notiflyClient(player, updateList)
            end
        end
    end
end

-- 事件更新
function EventCollectionMgr.update(player, moduleDatas, data, eventType, eventValType, param1, param2, param3)
    local updateList = {}

    for k, v in pairs(data) do
        local conf = Table_AchievementCondition[v]
        local ok = false
        local moduleData = moduleDatas[k]
        if conf ~= nil and eventType == conf.ConditionType and conf.Parameter[2] == param2 and conf.Parameter[3] == param3 and (type(moduleData) == "table") then
            if moduleData.val < conf.Parameter[1] then
                ok = true
            end

            if ok then
                if eventValType == EventCollectionMgr.EVENT_COLLECTION_VAL_TYPE.ADDUP then
                    moduleData.val = moduleData.val + param1

                else
                    moduleData.val = param1
                end

                updateList[k] = v
            end
        end
    end

    return updateList
end

EventCollectionMgr.regEventModule()




