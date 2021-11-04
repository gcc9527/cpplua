//#if defined(__GNUC__)
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
//#elif defined(_MSC_VER)
//#pragma warning(disable : 4996)
//#endif

#define _HAS_STD_BYTE 0


#include <unordered_map>
#include <thread>
#include <fstream>
#include <iostream>

#include "net/Data.h"
#include "net/TcpServer.h"
#include "codec/Codec.h"
#include "script/Script.h"
#include "player/PlayerMgr.h"
#include "MainThread.h"
#include "msgHandle/msgHandle.h"
#include "db/DbStorage.h"
#include "log/Log.h"
#include "cfg/Cfg.h"



extern std::unordered_map<int, MessageBaseInfo> gMessIdMapName;




int main()
{
	logInit("../log/", "log.log");
	initConfig();
	gMainThread.initMainThread();
	gScript.openLua();


	TcpServer tcp;
	tcp.start();


	//while (true)
	//{
	//	Task t;
	//	std::unique_lock<std::mutex> lk(gMutex);
	//	while (!gQueue.empty())
	//	{
	//		t = std::move(gQueue.front());
	//		gQueue.pop();
	//		break;
	//	}
	//	gCondVar.notify_one();
	//	lk.unlock();

	//	if (t.second_)
	//	{
	//		gScript.secondUpdate(); // 回调lua,每秒更新
	//	}
	//	else if (t.cmd_.size() > 0)
	//	{
	//		gScript.serverCmd(t.cmd_);
	//	}
	//	else if (t.message_.sessionId > 0 && t.message_.data_ != nullptr)
	//	{
	//		int messageId = 0;
	//		size_t fixLen = sizeof(int);
	//		memcpy(&messageId, t.message_.data_, fixLen);


	//		auto it = gMessIdMapName.find(messageId);
	//		if (it == gMessIdMapName.end())
	//		{
	//			logDebug("Script::onMessage no find name, messageId = %d", messageId);
	//		}
	//		else
	//		{
	//			if (gMainThread.isCppProcess(it->second.lang_, "cpp"))
	//			{
	//				msgHandle(messageId, t.message_.sessionId, it->second.name_, t.message_.data_ + fixLen, t.message_.lens_ - fixLen);
	//			}
	//			else
	//			{
	//				gScript.onMessage(messageId, t.message_.sessionId, it->second.name_, t.message_.data_ + fixLen, t.message_.lens_ - fixLen);
	//			}
	//		}

	//		free(t.message_.data_);
	//		t.message_.data_ = nullptr;
	//	}
	//	else if (t.clientOpt_.opt == 2)
	//	{
	//		gPlayerMgr.save(t.clientOpt_.sessionId);
	//	}
	//}

	std::cin.get();

	return 0;
}