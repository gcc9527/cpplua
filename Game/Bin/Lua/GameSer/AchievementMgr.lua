


-- 成就模块


local msg_handle = require "Base.MsgHandle"
local BaseUtil = require("Util.BaseUtil")
local PlayerData = require "Base.PlayerData"
local Table_AchievementCondition = require("Config.Table_AchievementCondition")
local Table_Achievement = require("Config.Table_Achievement")
local Table_FactionLevel = require("Config.Table_FactionLevel")
local Table_AchievementRewards = require("Config.Table_AchievementRewards")

AchievementMgr = {}





function AchievementMgr.getData(player)
    local allDatas = PlayerData.GetStaticData(player)
    return allDatas[PlayerData.PlayerStaticDataKey.Achievement]
end

function AchievementMgr.getDataForProcess(player)
    local allDatas = PlayerData.GetStaticData(player)
    if allDatas == nil then
        return nil
    end
    local datas = allDatas[PlayerData.PlayerStaticDataKey.Achievement]
    if datas ~= nil then
        return datas.achi
    end
    return nil
end

function AchievementMgr.regEvent()
    for k, v in ipairs(Table_Achievement) do
        local t = Table_AchievementCondition[v.MissId]
        if t ~= nil then
            EventCollectionMgr.regEvent(t.Id, k, t.ConditionType, 1)
        end
    end
end


function AchievementMgr.checkNewAchievement(player)
    --local datas = AchievementMgr.getData(player)
    --if datas == nil then
    --    return
    --end
    --
    --for k, v in pairs(Table_Achievement) do
    --    local data = datas.achi
    --    if data[k] == nil then
    --        data[k] =
    --        {
    --            val = 0, -- 进度值
    --            status = 0, -- 奖励领取状态
    --        }
    --
    --        local t = Table_AchievementCondition[v.MissId]
    --        if t ~= nil then
    --            EventCollectionMgr.regEvent(t.Id, k, t.ConditionType)
    --        end
    --    end
    --end


end

function AchievementMgr.reset(player, resetType)
    local datas = AchievementMgr.getData(player)
    if datas == nil then
        return
    end

    for k, v in pairs(Table_Achievement) do
        local data = datas.achi[k]
        if v.type == resetType and data ~= nil then

            data.val = 0
            data.status = 0
        end
    end

    local data = datas.act[resetType]
    if data ~= nil then
        data.val = 0
        for k, v in pairs(data.lists) do
            data.lists[k] = 0
        end
    end
end


function AchievementMgr.weekDay(player)
    --print("AchievementMgr.weekDay inAchievementMgr.weekDay inAchievementMgr.weekDay")
    AchievementMgr.reset(player, EventCollectionMgr.RESETTYPE.week)
    AchievementMgr.MsgAchievementReq(nil, player)
    --BaseUtil.printData(AchievementMgr.getDataForProcess(player))
end

function AchievementMgr.newDay(player)
    --print("AchievementMgr.newDay inAchievementMgr.newDay inAchievementMgr.newDay")
    AchievementMgr.checkNewAchievement(player)
    AchievementMgr.reset(player, EventCollectionMgr.RESETTYPE.days)
    AchievementMgr.MsgAchievementReq(nil, player)

    --BaseUtil.printData(AchievementMgr.getDataForProcess(player))
end

function AchievementMgr.onLogin(player)

    AchievementMgr.checkNewAchievement(player)
    AchievementMgr.MsgAchievementReq(nil, player)
    --local datas = AchievementMgr.getData(player)
    --BaseUtil.printData(datas.achi)

    --BaseUtil.printData(datas.fame)
    --AchievementMgr.MsgAchievementRecvRewardReq({id = 2}, player)

end


function AchievementMgr.notiflyClient(player, updateList)
    local msgs = {}
    msgs.MsgAchievementInfos = {}

    local achiData = AchievementMgr.getDataForProcess(player)
    local alchemistData = AlchemistMgr.getData(player)

    for k, v in pairs(updateList) do
        local data = achiData[k]
        local condConf = Table_AchievementCondition[v]

        if condConf ~= nil and data ~= nil then
            local msg = {}
            msg.id = k
            msg.val = data.val
            msg.status = 0

            if data.val >= condConf.Parameter[1] then
                msg.status = 1
                data.status = 1
            end

            table.insert(msgs.MsgAchievementInfos, msg)
            AlchemistMgr.updateRef(player, alchemistData, k, msg.status) -- 成就完成,更新炼制数据
        end
    end

    SendPacket2User(player, "MsgAchievementUpdateAck", msgs)
    AlchemistMgr.notiflyClient(player, alchemistData)

end

function AchievementMgr.init(player)
    local allDatas = PlayerData.GetStaticData(player)
    allDatas[PlayerData.PlayerStaticDataKey.Achievement] =
    {
        achi = {}, -- 成就数据
        act = {}, -- 活跃数据
        fame = {}, -- 声誉数据
    }
    local datas = allDatas[PlayerData.PlayerStaticDataKey.Achievement]

    for k, v in pairs(Table_Achievement) do
        datas.achi[k] =
        {
            val = 0, -- 进度值
            status = 0, -- 奖励领取状态
        }
    end

    for k, v in pairs(Table_AchievementRewards) do
        if datas.act[v.Type] == nil then
            datas.act[v.Type] =
            {
                val = 0, -- 当前活跃度
                lists = {}, -- 奖励领取列表
            }
        end

        datas.act[v.Type].lists[k] = 0
    end

    for k, v in pairs(Table_FactionLevel) do
        if datas.fame[v.ID] == nil then
            datas.fame[v.ID] =
            {
                lv = 0, -- 声誉等级
                val = 0, -- 声誉积分
            }
        end
    end

    return datas
end


function AchievementMgr.MsgAchievementReq(packet, player)
    local datas = AchievementMgr.getData(player)
    if datas == nil then
        datas = AchievementMgr.init(player)
    end

    local msgs =
    {

        MsgAchievementInfos = {},
        MsgAchievementActInfos = {},
        MsgAchievementFameInfos = {}
    }


    for k, v in pairs(datas.achi) do
        local achiMsg = {}
        achiMsg.id = k
        achiMsg.val = v.val
        achiMsg.status = v.status

        table.insert(msgs.MsgAchievementInfos, achiMsg)
    end


    for k, v in pairs(datas.act) do
        local actMsg = {}
        actMsg.actType = k
        actMsg.val = v.val
        actMsg.MsgAchievementActLists = {}

        for k1, v1 in pairs(v.lists) do
            table.insert(actMsg.MsgAchievementActLists, {id = k1, status = v1})
        end

        table.insert(msgs.MsgAchievementActInfos, actMsg)
    end

    for k, v in pairs(datas.fame) do
        local fameMsg = {}
        fameMsg.fameType = k
        fameMsg.lv = v.lv
        fameMsg.val = v.val

        table.insert(msgs.MsgAchievementFameInfos, fameMsg)
    end

    SendPacket2User(player, 'MsgAchievementAck', msgs)
end


function AchievementMgr.MsgAchievementRecvRewardReq(packet, player)
    local datas = AchievementMgr.getData(player)
    local playerId = C_GetObjId(player)
    if datas == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvRewardReq datas == nil, playerId = ", playerId)
        return
    end

    local achievementId = packet.id
    if achievementId == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvRewardReq achievementId == nil, playerId = ", playerId)
        return
    end

    local conf = Table_Achievement[achievementId]
    if conf == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvRewardReq conf == nil, playerId = ", playerId)
        return
    end

    local data = datas.achi[achievementId]
    if data == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvRewardReq data == nil, playerId = ", playerId)
        return
    end

    if data.status ~= 1 then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvRewardReq data.status ~= 1, playerId = ", playerId)
        return
    end


    if not ItemMgr.checkReward(conf.ItemID, conf.ItemNum) then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvRewardReq idCnt ~= numCnt, playerId = ", playerId)
        return
    end

    ItemMgr.addItemList(player, conf.ItemID, conf.ItemNum, OP_ITEM_IDLE_ACHIEVEMENT, 0)

    data.status = 2

    local msgs =
    {
        MsgAchievementInfos =
        {
            id = achievementId,
            val = data.val,
            status = data.status
        },
        MsgAchievementActInfos = {},
        MsgAchievementFameInfos = {}
    }

    local actData = datas.act[conf.type]
	msgs.MsgAchievementActInfos.actType = conf.type
	msgs.MsgAchievementActInfos.val = 0
	msgs.MsgAchievementActInfos.MsgAchievementActLists = {}
    if actData ~= nil and conf.ActivityPoint ~= -1 then
        actData.val = actData.val + conf.ActivityPoint
        msgs.MsgAchievementActInfos.val = actData.val

        for k, v in pairs(Table_AchievementRewards) do
            if v.Type == conf.type then
                local val = actData.lists[k]
                if val ~= nil then
                    if actData.val >= v.Point and val == 0 then
                        actData.lists[k] = 1
                    end

                    local msg = {}
                    msg.id = k
                    msg.status = val

                    table.insert(msgs.MsgAchievementActInfos.MsgAchievementActLists, msg)
                end
            end
        end
    end

    local fameData = datas.fame[conf.faction]
    if fameData ~= nil then
        fameData.val = fameData.val + conf.AchievePoint
        local levelUpFlag = 0
        local iMaxFactLevel = 0
        for k, v in ipairs(Table_FactionLevel) do
            if v.ID == conf.faction and fameData.lv == v.FactionLevel then
                if fameData.val >= v.NextLevel then					
                    fameData.lv = fameData.lv + 1
					levelUpFlag = levelUpFlag + 1
                end

                msgs.MsgAchievementFameInfos.fameType = conf.faction
                msgs.MsgAchievementFameInfos.lv = fameData.lv
                msgs.MsgAchievementFameInfos.val = fameData.val
            end
            if v.ID == conf.faction and iMaxFactLevel < v.FactionLevel then
                iMaxFactLevel = v.FactionLevel
            end
        end

        if fameData.lv - levelUpFlag >= iMaxFactLevel then--满级之后就不用走下面的增加特权了
            levelUpFlag = 0
        end

        if fameData.lv > iMaxFactLevel then
            fameData.lv = iMaxFactLevel
        end
		
		if levelUpFlag > 0 then
			for k, v in ipairs(Table_FactionLevel) do
				if v.ID == conf.faction and fameData.lv == v.FactionLevel then
					if v.PrivilegeId > 0 then
					    --BaseUtil.PrintEx("MsgAchievementRecvRewardReq v.PrivilegeId=", v.PrivilegeId)
						PrivilegeMgr.update(player, conf.faction, v.PrivilegeId)
					end
				end
			end
		end
		
		
    end
    SendPacket2User(player, 'MsgAchievementRecvRewardAck', msgs)
end


function AchievementMgr.MsgAchievementRecvActRewardReq(packet, player)
    local actType, actId = packet.actType, packet.id
    if actType == nil or actId == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq actType == nil or actId == nil")
        return
    end

    local conf = nil
    for k, v in pairs(Table_AchievementRewards) do
        if v.Type == actType and actId == k then
            conf = v
            break
        end
    end

    if conf == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq conf == nil")
        return
    end

    local datas = AchievementMgr.getData(player)
    if datas == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq datas == nil")
        return
    end

    local actData = datas.act
    if actData == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq datas == nil")
        return
    end

    local typeData = actData[actType]
    if typeData == nil then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq typeData == nil")
        return
    end

    local status = typeData.lists[actId]
    if status ~= 1 then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq status ~= 1")
        return
    end

    if typeData.val < conf.Point then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq typeData.val < conf.Point")
        return
    end

    if not ItemMgr.checkReward(conf.ItemID, conf.ItemNum) then
        BaseUtil.PrintEx("AchievementMgr.MsgAchievementRecvActRewardReq ItemMgr.checkReward err")
        return
    end

    ItemMgr.addItemList(player, conf.ItemID, conf.ItemNum, OP_ITEM_IDLE_ACHIEVEMENT_ACT, 0)
    typeData.lists[actId] = 2

    SendPacket2User(player, "MsgAchievementRecvActRewardAck", {actType = actType, id = actId, status = 2})
end




EventCollectionMgr.regDataFunc(1, AchievementMgr.getDataForProcess)
AchievementMgr.regEvent()


function AchievementMgr.checkCom(player, achiId)
    local datas = AchievementMgr.getData(player)
    if datas == nil then
        BaseUtil.PrintEx("AchievementMgr.checkCom(player, achiId) datas == nil, id = ", achiId)
        return false
    end

    local data = datas.achi[achiId]
    if data == nil then
        BaseUtil.PrintEx("AchievementMgr.checkCom(player, achiId) data == nil, id = ", achiId)
        return false
    end

    local conf = Table_Achievement[achiId]
    if conf == nil then
        BaseUtil.PrintEx("AchievementMgr.checkCom(player, achiId) conf == nil, id = ", achiId)
        return false
    end

    local condConf = Table_AchievementCondition[conf.MissId]
    if condConf == nil then
        BaseUtil.PrintEx("AchievementMgr.checkCom(player, achiId) condConf == nil, id = ", achiId)
        return false
    end

    if data.val < condConf.Parameter[1] or 1 then
        return false
    end

    return true
end


msg_handle.register('MsgAchievementReq', AchievementMgr.MsgAchievementReq)
msg_handle.register('MsgAchievementRecvRewardReq', AchievementMgr.MsgAchievementRecvRewardReq)
msg_handle.register('MsgAchievementRecvActRewardReq', AchievementMgr.MsgAchievementRecvActRewardReq)