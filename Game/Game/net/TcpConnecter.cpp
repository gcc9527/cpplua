#include "TcpConnecter.h"

#ifdef __linux__
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <thread>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "../log/Log.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Data.h"



std::queue<Task> gQueue;
std::mutex gMutex;
std::condition_variable gCondVar;
constexpr size_t gMaxTask = 50;
extern std::mutex gMainMutex;


TcpConnecter::TcpConnecter(int fd, EventLoop* eventLoop) :
	writeing_(false),
	fd_(fd),
	sessionId_(0),
	ip_(""),
	port_(0),
	eventLoop_(eventLoop),
	channel_(nullptr)
{

}

TcpConnecter::~TcpConnecter()
{
	if (channel_)
	{
		delete channel_;
		channel_ = nullptr;
	}
	eventLoop_ = nullptr;
}

void TcpConnecter::initcon(struct sockaddr_in* addr)
{
	ip_ = inet_ntoa(addr->sin_addr);
	port_ = ntohs(addr->sin_port);

	channel_ = new Channel(fd_);
	channel_->setReadCb(std::bind(&TcpConnecter::onRead, this));
	channel_->setWriteCb(std::bind(&TcpConnecter::onWrite, this));
#ifdef _WIN32
	eventLoop_->updateEvent(channel_, eventLoop_->getReads());
#else
	eventLoop_->updateEvent(channel_, nullptr);
#endif

}


void TcpConnecter::nowWrite(char* data, size_t lens)
{
	while (true)
	{
#ifdef _WIN32
		int ret = send(fd_, data, (int)lens, 0);
#else
		int ret = send(fd_, data, lens, 0);
#endif
		if (ret < 0)
		{
#ifdef _WIN32
			logDebug("TcpConnecter::nowWrite err win, code = %d", WSAGetLastError());
			onClose();
			return;
#else
			if (ret != EWOULDBLOCK || ret != EAGAIN)
			{
				perror("TcpConnecter::nowWrite err linux");
				onClose();
				return;
			}
			else
			{
				continue;
			}
#endif
		}
		else
		{
			if (ret < lens)
			{
				int leftLen = (int)lens - ret;
				writeBuff_.copyData(data, leftLen);

#ifdef _WIN32
				eventLoop_->updateEvent(channel_, &eventLoop_->writes_, 1);
#else
				channel_->setEvent(EPOLLIN | EPOLLOUT);
				eventLoop_->updateEvent(channel_, nullptr, 3);
#endif
				updateWriteing(true);
			}
		}
		return;
	}
}

void TcpConnecter::parseMessage()
{
	size_t bodyLen = 0;

	size_t fixlen = sizeof(int);
	memcpy(&bodyLen, readBuff_.getHeadPos(), fixlen);

	logDebug("TcpConnecter::parseMessage, bodyLen = %d", bodyLen);

	if (bodyLen <= 0)
	{
		logDebug("Buffer::parseHead bodylen <= 0");
		return;
	}

	size_t readPos = readBuff_.readPos();

	size_t dataCurPos = fixlen + readBuff_.writePos();
	if (readPos - dataCurPos >= bodyLen) // 一个完整的消息包
	{
		std::unique_lock<std::mutex> lk(gMutex);
		while (gQueue.size() >= gMaxTask)
		{
			gCondVar.wait(lk);
		}

		Task t;
		char* buf = (char*)malloc(bodyLen);
		memcpy(buf, readBuff_.data() + dataCurPos, bodyLen);

		t.message_.data_ = buf;
		t.message_.lens_ = bodyLen;
		t.message_.sessionId = sessionId_;
		logDebug("client push");

		gQueue.emplace(std::move(t));
		lk.unlock();

		readBuff_.updateWritePos(bodyLen + fixlen);// 更新解析数据包wpos的位置(包头固定4b+计算得到的边长包体
		size_t maxLen = readBuff_.size();
		size_t wPos = readBuff_.writePos();
		if (wPos == readBuff_.readPos()) // 全部解析完了
		{
			readBuff_.reset();
		}
		else if (maxLen - wPos <= 50) // 解析完后可重复使用的空间太多了
		{
			readBuff_.moveBuff();
		}
	}
}


void TcpConnecter::onRead()
{
	logDebug("work thread recv data, tid = %d, ip = %s, port = %d", std::this_thread::get_id(), ip_, port_);
	readBuff_.expand();
#ifdef _WIN32
	int ret = recv(fd_, readBuff_.data(), (int)readBuff_.leftSpace(), 0);
#else
	int ret = recv(fd_, readBuff_.data(), readBuff_.leftSpace(), 0);
#endif
	if (ret < 0)
	{

#ifdef _WIN32
		logDebug("TcpConnecter::onRead err win, code = %d", WSAGetLastError());
		onClose();
		return;
#else
		if (ret != EWOULDBLOCK || ret != EAGAIN)
		{
			perror("TcpConnecter::onRead linux");
			onClose();
			return;
		}
#endif
	}
	else if (ret == 0)
	{
		onClose();
	}
	else
	{
		readBuff_.updateReadPos(ret);
		parseMessage();
	}
}

void TcpConnecter::onWrite()
{
	logDebug("work thread write data, tid = %d", std::this_thread::get_id());
	size_t validLen = writeBuff_.validWriteLen();
#ifdef _WIN32
	int ret = send(fd_, writeBuff_.writeData(), (int)validLen, 0);
#else
	int ret = send(fd_, writeBuff_.writeData(), validLen, 0);
#endif
	if (ret < 0)
	{

#ifdef _WIN32
		logDebug("TcpConnecter::onRead err win, code = %d", WSAGetLastError());
		onClose();
		return;
#else
		if (ret != EWOULDBLOCK || ret != EAGAIN)
		{
			perror("TcpConnecter::onRead linux");
			onClose();
			return;
		}
#endif
	}
	else
	{
		if (ret == validLen)
		{
			updateWriteing(false);

			bool ok = true;
			WriteTask t;

			gMainMutex.lock();
			if (!eventLoop_->queue_.empty())
			{
				ok = false;
			}
			gMainMutex.unlock();

			if (ok)
			{
#ifdef _WIN32
				eventLoop_->updateEvent(channel_, eventLoop_->getWrites(), 2);
#else
				channel_->setEvent(EPOLLIN);
				eventLoop_->updateEvent(channel_, nullptr, 3);
#endif
			}
		}

		writeBuff_.updateWritePos(ret);
		if (writeBuff_.readPos() == writeBuff_.writePos())
		{
			writeBuff_.reset();
		}

		if (writeBuff_.size() - writeBuff_.writePos() <= 50) 
		{
			writeBuff_.moveBuff();
		}
	}
}

void TcpConnecter::onClose()
{
	if (!channel_->getCloseStatus())
	{
		channel_->setCloseStatus(true);
		return;
	}

#ifdef _WIN32
	eventLoop_->updateEvent(channel_, eventLoop_->getReads(), 2);
	eventLoop_->updateEvent(channel_, eventLoop_->getWrites(), 2);
#else
	eventLoop_->updateEvent(channel_, nullptr, 2);
#endif


#ifdef _WIN32
	closesocket(fd_);
#else
	close(fd_);
#endif

	if (closeCb_)
	{
		closeCb_(sessionId_);
	}
}
