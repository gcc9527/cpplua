

testMgr = {}


function testMgr.test(player, msgs)
    tools.pt(msgs)

    local msgs = {}
    msgs.id = 1
    msgs.name = "myname"
    msgs.age = 20

    gMainThread:sendMessage(1, "Person", msgs)
end
