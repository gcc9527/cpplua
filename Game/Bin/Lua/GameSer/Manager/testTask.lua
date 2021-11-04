
testMgr = {}


local function getData(player)

end

local testTbale =
{
    [1] ={Id=1,taskId = 1},
    [2] ={Id=2,taskId = 3}
}

local taskTable =
{
    [1]={Id = 1, Type = 7, p1=1, p2=2, p3=3},
    [2]={Id = 2, Type = 3, p1=-1,p2=-1, p3=-1},
    [3]={Id = 3, Type = 4, p1=1, p2=-1, p3=-1}
}

function testMgr.getDataForProcess(player)
    local datas = getData(player)
    if datas ~= nil then
        return datas.task
    end
    return nil
end

function testMgr.regEvent()
    for k, v in ipairs(testTbale) do
        local t = taskTable[v.taskId]
        if t ~= nil then
            eventCollectMgr.regEvent(t.Id, k, t.Type, 1)
        end
    end
end


function testMgr.notiflyClient(player, updateList)
    
end

function testMgr.init(player)
    local datas = getData(player)
    for k, v in pairs(testTbale) do
        datas.task[k] =
        {
            val = 0, -- 进度值
        }
    end
end


eventCollectMgr.regDataFunc(1, testMgr.getDataForProcess)
testMgr.regEvent()




