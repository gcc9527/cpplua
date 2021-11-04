#pragma once



#include <functional>


class TcpConnecter;

class Channel
{
public:
	Channel(int fd);
	~Channel();

public:
	typedef std::function<void()> EventCallBack;

	inline void addEvent(int event) { event_ = event; }
	inline int getEvent() { return event_; }
	inline int getFd() { return fd_; }
	inline void setEvent(int event) { revent_ = event; }
	inline void setTcpConnecter(TcpConnecter* con) { con_ = con; }
	inline bool getCloseStatus() { return isClose_; }
	inline void setCloseStatus(bool status) { isClose_ = status; }
	void onEvent();
	void setReadCb(EventCallBack cb) { readCb_ = std::move(cb); }
	void setWriteCb(EventCallBack cb) { writeCb_ = std::move(cb); }

private:
	int fd_;
	int event_; // 读事件
	int revent_; // 写事件
	bool isClose_;
	TcpConnecter* con_;

	EventCallBack readCb_;
	EventCallBack writeCb_;
};

