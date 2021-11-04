#pragma once



#include <functional>

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
	void onEvent();
	void setReadCb(EventCallBack cb) { readCb_ = std::move(cb); }
	void setWriteCb(EventCallBack cb) { writeCb_ = std::move(cb); }

private:
	int fd_;
	int event_; // 读事件
	int revent_; // 写事件

	EventCallBack readCb_;
	EventCallBack writeCb_;
};

