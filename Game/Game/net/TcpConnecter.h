#pragma once





#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>

#endif

#include <string>
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
	inline EventLoop* getEventLoop() { return eventLoop_; }
	void nowWrite(char* data, size_t lens);
	void updateWriteing(bool writeing = false) { writeing_ = writeing; }
	bool isWriteing() { return writeing_; }
	inline void setSessionId(int sessionId) { sessionId_ = sessionId; }

private:
	void onRead();
	void onWrite();
	void onClose();
	void parseMessage();

private:
	bool writeing_;
	int fd_;
	int sessionId_;
	const char* ip_;
	uint16_t port_;
	EventLoop* eventLoop_;
	Channel* channel_;
	EventCallBack closeCb_;
	Buffer readBuff_;
	Buffer writeBuff_;

};

