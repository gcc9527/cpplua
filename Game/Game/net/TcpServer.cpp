
#include "TcpServer.h"


#ifdef _WIN32
#include <windows.h>
#include<ws2tcpip.h>
#else
#include <sys/syscall.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <string.h>

#include <string>
#include <iostream>
#include <chrono>
#include <condition_variable>

#include "../log/Log.h"
#include "Nettools.h"
#include "EventLoop.h"
#include "TcpConnecter.h"
#include "Data.h"



extern constexpr size_t gMaxTask = 10;
int gSessionId = 1;

#ifdef _WIN32
std::unordered_map<int, Channel*> gChannels;
#endif

std::unordered_map<int, TcpConnecter*> gClients;
std::mutex gMainMutex;



TcpServer::TcpServer()
{
	mainEventLoop_ = new EventLoop;
}

TcpServer::~TcpServer()
{
	if (mainEventLoop_)
	{
		delete mainEventLoop_;
		mainEventLoop_ = nullptr;
	}

	for (auto& e : workLoops_)
	{
		delete e;
		e = nullptr;
	}

}



void TcpServer::sendMessage(int sessionId, int messageId, char* data, size_t lens)
{
	TcpConnecter* con = nullptr;
	gMainMutex.lock();
	auto it = gClients.find(sessionId);
	if (it == gClients.end())
	{
		logDebug("TcpServer::sendMessage err, no find messageId = %lld", sessionId);
		gMainMutex.unlock();
		return;
	}

	con = it->second;
	gMainMutex.unlock();

	size_t fixLen = sizeof(int);
	size_t pos = fixLen * 2;
	size_t maxLen = pos + lens;

	char* buf = (char*)malloc(maxLen);
	size_t bodyLen = fixLen + lens;

	memcpy(buf, &bodyLen, fixLen);
	memcpy(buf + fixLen, &messageId, fixLen);
	memcpy(buf + pos, data, lens);

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








void TcpServer::start()
{
	mainThread_ = std::move(std::thread([&]()
		{
#ifdef _WIN32
			mainEventLoop_->updateEvent(&mainChannel_, mainEventLoop_->getReads());
#else
			mainEventLoop_->updateEvent(&mainChannel_, nullptr, EPOLL_CTL_ADD);
#endif
			mainChannel_.setReadCb(std::bind(&TcpServer::acceptClient, this));
			mainEventLoop_->startLoop();
		}));

	sleepThread_ = std::move(std::thread([]()
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				std::unique_lock<std::mutex> lk(gMutex);
				while (gQueue.size() >= gMaxTask)
				{
					gCondVar.wait(lk);
				}
				gQueue.emplace(Task(true));
				lk.unlock();
			}
		}));

	cmdThread_ = std::move(std::thread([]()
		{
			while (true)
			{
				std::string buf;
				std::getline(std::cin, buf);
				std::unique_lock<std::mutex> lk(gMutex);
				while (gQueue.size() >= gMaxTask)
				{
					gCondVar.wait(lk);
				}

				Task task;
				task.cmd_ = buf;
				gQueue.emplace(std::move(task));
				lk.unlock();
			}
		}));

	for (uint8_t i = 0; i < threadCnt_; ++i)
	{
		threadPool_.emplace_back(std::thread(&TcpServer::initWorkThread, this));
	}
#ifdef _WIN32
	gChannels.emplace(monitor_.getListenFd(), &mainChannel_);
#endif

}




void TcpServer::initWorkThread()
{
	EventLoop evevtLoop;
	mutex_.lock();
	workLoops_.emplace_back(&evevtLoop);
	mutex_.unlock();
	evevtLoop.startLoop();
}

void TcpServer::acceptClient()
{
	EventLoop* loop = nullptr;
	if (workLoops_.size() <= 0)
	{
		loop = mainEventLoop_;
	}
	else
	{
		loop = workLoops_[loopIndex++];
		if (loopIndex == (workLoops_.size() - 1))
		{
			loopIndex = 0;
		}
	}

	struct sockaddr_in addr;

	socklen_t len = sizeof(addr);
	int listenFd = monitor_.getListenFd();
	int cliFd = (int)accept(listenFd, (struct sockaddr*)&addr, &len);
	bool ok = true;

#ifdef _WIN32
	if (cliFd == INVALID_SOCKET)
	{
		logErr("accept err win, code = %d", WSAGetLastError());
		ok = false;
	}
#else
	if (cliFd < 0)
	{
		logErr("accept err linux");
		perror("accept");
		ok = false;
	}
#endif

	if (!ok)
	{
		return;
	}

	logInfo("main net work acceptClient, tid= %d, ip = %s,port = %d", std::this_thread::get_id(), inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	TcpConnecter* con = new TcpConnecter(cliFd, loop);
	con->initcon(&addr);
	con->setCloseCb(std::bind(&TcpServer::closeClient, this, std::placeholders::_1));
#ifdef _WIN32
	gChannels.emplace(cliFd, con->getChannel());
#endif

	int sessionId = gSessionId;
	gMainMutex.lock();
	if (!freeSessionIds_.empty())
	{
		sessionId = freeSessionIds_.back();
		freeSessionIds_.pop_back();
	}
	else
	{
		++gSessionId;
	}

	gClients.emplace(sessionId, con);
	gMainMutex.unlock();

	con->setSessionId(sessionId);
}

void TcpServer::closeClient(int sessionId)
{
	logDebug("TcpServer::closeClient , id = %d", std::this_thread::get_id());
	TcpConnecter* con = nullptr;

	gMainMutex.lock();
	auto it = gClients.find(sessionId);
	if (it == gClients.end())
	{
		logErr("TcpServer::closeClient no find this clint, sessionId =  %d", sessionId);
		gMainMutex.unlock();
		return;
	}
	con = it->second;
	freeSessionIds_.emplace_back(sessionId);
	gClients.erase(it);
	gMainMutex.unlock();

#ifdef _WIN32
	auto chit = gChannels.find(con->getChannel()->getFd());
	if (chit != gChannels.end())
	{
		gChannels.erase(chit);
	}
#endif
	delete con;

	std::unique_lock<std::mutex> lk(gMutex);
	while (gQueue.size() >= gMaxTask)
	{
		gCondVar.wait(lk);
	}

	Task t;
	t.clientOpt_.sessionId = sessionId;
	t.clientOpt_.opt = 2;
	gQueue.push(std::move(t));
	lk.unlock();

}
