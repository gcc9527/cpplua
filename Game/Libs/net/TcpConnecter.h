#pragma once





#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>

#endif


#include <functional>



#include "Buffer.h"

class EventLoop;
class Channel;

class TcpConnecter
{
public:
	TcpConnecter(int fd, EventLoop* eventLoop);
	~TcpConnecter();

public:
	typedef std::function<void(int)> EventCallBack;

	void initcon(struct sockaddr_in* addr);
	void setCloseCb(EventCallBack cb) { closeCb_ = std::move(cb); }
	inline Channel* getChannel() { return channel_; }

private:
	void onRead();
	void onWrite();
	void onClose();
	void parseMessage();

private:
	int fd_;
	const char* ip_;
	short port_;
	EventLoop* eventLoop_;
	Channel* channel_;
	EventCallBack closeCb_;
	Buffer readBuff_;
	Buffer writeBuff_;

};

