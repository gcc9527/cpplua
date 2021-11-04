

#pragma once

#include <stdint.h>


#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#pragma comment(lib,"ws2_32.lib ")
const uint8_t DEFAULTWORDTHREAD = 0;
#else
const uint8_t DEFAULTWORDTHREAD = 4;
#endif


#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include "Monitor.h"
#include "Channel.h"



class TcpConnecter;

constexpr uint8_t MAXWORDTHREAD = 10;


class EventLoop;
class TcpConnecter;

class TcpServer
{

public:
	TcpServer();
	~TcpServer();



public:
	void start();
	inline void setThreadCnt(uint8_t cnt)
	{
		if (cnt > MAXWORDTHREAD)
			cnt = MAXWORDTHREAD;
		threadCnt_ = cnt;
	}
	void sendMessage(int sessionId, int messageId, char* data, size_t lens);
private:
	typedef std::function<void()> EventCallBack;
	void initWorkThread();
	void setAcceptCb(EventCallBack cb) { acceptCb_ = std::move(cb); }
	void acceptClient();
	void closeClient(int sessionId);

private:
	uint8_t threadCnt_ = DEFAULTWORDTHREAD;
	size_t loopIndex = 0;
	Monitor monitor_;
	EventLoop* mainEventLoop_ = nullptr;
	Channel mainChannel_ = monitor_.getListenFd();
	std::thread mainThread_;
	std::thread sleepThread_;
	std::thread cmdThread_;
	std::mutex mutex_;
	std::vector<std::thread> threadPool_;
	std::vector<EventLoop*> workLoops_;
	std::vector<int> freeSessionIds_;
	EventCallBack acceptCb_;
};

