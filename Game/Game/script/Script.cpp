#include "Script.h"

#include "../MainThread.h"
#include "../player/Player.h"
#include "../player/PlayerMgr.h"
#include "../log/Log.h"
#include "../cfg/Cfg.h"

class Player;

extern std::unordered_map<int, MessageBaseInfo> gMessIdMapName;
extern std::unordered_map<const char*, MessageBaseInfo> gNameMapMessId;


lua_State* gLuaSgate = NULL;


Script::Script()
{

}

Script::~Script()
{

}


void Script::doFile(const char* path)
{
	lua_pushcclosure(gLuaSgate, LuaBind::errFunc, 0);
	int stackTop = lua_gettop(gLuaSgate);

	if (luaL_loadfile(gLuaSgate, path) == 0)
	{
		if (lua_pcall(gLuaSgate, 0, 0, stackTop))
		{
			lua_pop(gLuaSgate, 1);
		}
	}
	else
	{
		logErr("dofile error: %s", lua_tostring(gLuaSgate, -1));
		lua_pop(gLuaSgate, 1);

	}

	lua_pop(gLuaSgate, 1);

}

void Script::openLua()
{
	gLuaSgate = luaL_newstate();
	if (!gLuaSgate)
	{
		logErr("Script::openLua !gLuaSgate");
		exit(0);
	}
	luaL_openlibs(gLuaSgate);
	doFile("Lua/GameSer/testmain.lua");
	reg();
	doFile("Lua/GameSer/main.lua");

	LuaBind::CallLua cl("testTabfunc1");
	cl.call<std::map<int, int>, void>(1,2);

}

void Script::closeLua()
{
	lua_close(gLuaSgate);
}



void Script::onMessage(int messageId, int sessionId, const char* name, char* data, size_t lens)
{
	Player* player = gPlayerMgr.findPlayer(sessionId);
	if (!player)
	{
		logErr("Script::onMessage no find player");
		return;
	}

	//Player* player = new Player();
	char buf[30] = { 0 };
	sprintf(buf, "msgHandle_%d", messageId);
	LuaBind::CallLua cl(buf);
	if (cl.existFunc())
	{
		cl.call<void, LuaMsg>(player, name, data, lens);
	}
	else
	{
		logErr("no find %s", buf);
	}



}

void Script::secondUpdate()
{
	//LuaBind::CallLua cl("gServerUpdate");
	//cl.call<void, void>((long long)time(0));
}

void Script::serverCmd(const std::string& cmd)
{
	LuaBind::CallLua cl("gServerCmd");
	cl.call<void, void>(cmd);

}

void luaLog(int lv, const char* txt)
{
	switch (lv)
	{
	case 0: // fatal
		logFatal("%s", txt);
		return;
	case 1: // err
		logErr("%s", txt);
		return;
	case 2: // info
		logInfo("%s", txt);
		return;
	case 3: // debug
		logDebug("%s", txt);
		return;
	default:
		return;
	}

}





void Script::reg()
{
#ifdef _WIN32
#define ENUM_PLATFORM_WIN 1
	Reg_Global_Var_Same(ENUM_PLATFORM_WIN);
#else
#define ENUM_PLATFORM_LINUX 1
	MLUA_REG_NUM(ENUM_PLATFORM_LINUX);
#endif


	Reg_Gloabl_Func(luaLog);
	Reg_Gloabl_Func(initConfig);

	Reg_Class(MainThread);
	Reg_Global_Var_No_Same(&gMainThread, "gMainThread");
	Reg_Class_Func(MainThread, sendMessage);
	Reg_Class_Func(MainThread, parseProto);
	Reg_Class_Func(MainThread, cacheMessage);
	Reg_Class_Func(MainThread, f1);
	Reg_Class_Func(MainThread, teststring);

	Reg_Class(PlayerMgr);
	Reg_Global_Var_No_Same(&gPlayerMgr, "gPlayerMgr");
	Reg_Class_Func(PlayerMgr, findPlayer);


	Reg_Class(Player);
	Reg_Class_Func(Player, getSessionId);
	Reg_Class_Func(Player, getUid);
	Reg_Class_Func(Player, getBagMgr);


	Reg_Class(BagMgr);
	Reg_Class_Func(BagMgr, isFullById);
	Reg_Class_Func(BagMgr, isFullByUid);
	Reg_Class_Func(BagMgr, addItem);

	Reg_Class_Func(BagMgr, addItemList);
	Reg_Class_Func(BagMgr, removeItemByUid);
	Reg_Class_Func(BagMgr, removeItemById);
	Reg_Class_Func(BagMgr, removeItemListByUid);
	Reg_Class_Func(BagMgr, removeItemListById);
	Reg_Class_Func(BagMgr, getItemCntById);
	Reg_Class_Func(BagMgr, getItemCntByUid);
	Reg_Class_Func(BagMgr, isEnoughById);
	Reg_Class_Func(BagMgr, isEnoughByUid);
	Reg_Class_Func(BagMgr, isEnoughListById);
	Reg_Class_Func(BagMgr, isEnoughListByUId);



}

