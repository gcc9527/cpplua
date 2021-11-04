#include "MainThread.h"


#include <unordered_map>

#include "net/EventLoop.h"
#include "net/TcpConnecter.h"
#include "log/Log.h"
#include "db/DbStorage.h"
#include "public/ExitThread.hpp"



std::unordered_map<int, MessageBaseInfo> gMessIdMapName;
std::unordered_map<const char*, MessageBaseInfo> gNameMapMessId;

extern google::protobuf::compiler::DiskSourceTree g_sourceTree;
extern google::protobuf::compiler::Importer g_importer;
extern google::protobuf::DynamicMessageFactory      g_factory;


extern std::mutex gMainMutex;
extern std::unordered_map<int, TcpConnecter*> gClients;


MainThread::MainThread()
{
	dbThread_ = std::thread(&DbStorage::save, &DbStorage::instance());
}

MainThread::~MainThread()
{

}


void MainThread::parseProto(const char* protoName)
{

	if (!g_importer.Import(protoName))
	{
		logDebug("parseProto err, name = %s", protoName);
		ExitPro::exitProcess();
	}
}


void MainThread::cacheMessage(int messageId, const char* lang, const char* name, const char* desc)
{
	gMessIdMapName.emplace(messageId, MessageBaseInfo(messageId, lang, name));
	gNameMapMessId.emplace(name, MessageBaseInfo(messageId, lang, name));
}


DbTask* MainThread::f1()
{
	return new DbTask;
}


void MainThread::teststring(std::string s)
{
	std::cout << s << std::endl;
}

bool MainThread::isCppProcess(const char* dest, const char* src)
{
	if (strncmp(dest, src, 3) == 0)
	{
		return true;
	}
	return false;
}

void MainThread::initMainThread()
{
	g_sourceTree.MapPath("", "../Protobuf/");
}

void MainThread::sendMessage(int sessionId, const char* name, LuaMsg& luaMsg)
{
	if (!luaMsg.data_)
	{
		logErr("sendMessage no find data, name = %s", name);
		return;
	}

	auto messit = gNameMapMessId.find(name);
	if (messit == gNameMapMessId.end())
	{
		logDebug("sendMessage no find data, name = %s", name);
		return;
	}


	int messageId = messit->second.messageId_;
	size_t lens = luaMsg.lens_;

	TcpConnecter* con = nullptr;
	gMainMutex.lock();
	auto cit = gClients.find(sessionId);
	if (cit == gClients.end())
	{
		printf("sendMessage err, no find messageId = %d", sessionId);
		gMainMutex.unlock();
		return;
	}

	con = cit->second;
	gMainMutex.unlock();

	size_t fixLen = sizeof(int);
	size_t pos = fixLen * 2;
	size_t maxLen = pos + luaMsg.lens_;

	char* buf = (char*)malloc(maxLen);
	size_t bodyLen = fixLen + lens;

	memcpy(buf, &bodyLen, fixLen);
	memcpy(buf + fixLen, &messageId, fixLen);
	memcpy(buf + pos, luaMsg.data_, lens);

	EventLoop* loop = con->getEventLoop();
	WriteTask t;
	t.sessionId = sessionId;
	t.data_ = buf;
	t.lens = maxLen;

	loop->writeMutex_.lock();
	loop->queue_.emplace(std::move(t));
	loop->writeMutex_.unlock();


#ifdef __linux__
	loop->weakUp();
#endif








}



