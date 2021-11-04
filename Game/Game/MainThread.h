#pragma once


#include <thread>
#include <queue>
#include "public/Lua.hpp"
#include "public/Singleton.h"

class Player;

struct MessageBaseInfo
{
	MessageBaseInfo(int messageId, const char* lang, const char* name) :
		messageId_(messageId),
		lang_(lang),
		name_(name)
	{

	}

	int messageId_;
	const char* lang_;
	const char* name_;
};


struct DbTask
{
	int sessionId_ = 0;
	int serverId_ = 0;
	const char* tableName_ = nullptr;
	const char* data_ = nullptr;
	const char* fieldName_ = nullptr;
	Player* player_ = nullptr;
};

class MainThread : public Singleton<MainThread>
{
public:
	MainThread();
	~MainThread();
public:
	bool isCppProcess(const char* dest, const char* src);
	void initMainThread();
	void parseProto(const char* protoName);
	void sendMessage(int sessionId, const char* name, LuaMsg& luaMsg);
	void cacheMessage(int messageId, const char* lang, const char* name, const char* desc);
	DbTask* f1();
	void teststring(std::string s);

private:
	std::thread dbThread_;

public:
	std::mutex mutex_;
	std::condition_variable cond_;
	std::queue<DbTask> queueTask_;
};


#define gMainThread MainThread::instance()