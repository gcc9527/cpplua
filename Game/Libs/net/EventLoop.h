#pragma once




#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <sys/syscall.h>
#include <sys/epoll.h>
#endif
#include <unordered_map>

class Channel;
//extern std::unordered_map<int, Channel*> gChannel;

#include <vector>




class EventLoop
{
public:
	EventLoop();
	~EventLoop();

public:
	void updateEvent(Channel* channel, fd_set* fds, int opt = 0);
	void startLoop();


#ifdef _WIN32
	inline fd_set* getReads() { return &reads_; }
	inline fd_set* getWrites() { return &writes_; }
	inline fd_set* getExcepts() { return &excepts_; }
	void processReads(fd_set* fds);
	void processWrites(fd_set* fds);
	void processExcepts(fd_set* fds);
#endif


#ifdef _WIN32
	fd_set reads_;
	fd_set writes_;
	fd_set excepts_;
#else
	int efd_ = -1;
	std::vector<struct epoll_event> eventList_;

#endif
};

