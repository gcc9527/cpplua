#pragma once

#include "../public/Lua.hpp"
#include "../public/Singleton.h"
class Player;

class Script : public Singleton<Script>
{
public:
	Script();
	~Script();
public:
	void openLua();
	void closeLua();
	void doFile(const char* path);
	void onMessage(int messageId, int sessionId, const char* name, char* data, size_t lens);

	void secondUpdate();
	void reg();
	void serverCmd(const std::string& cmd);


};


#define gScript Script::instance()