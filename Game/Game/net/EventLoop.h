#pragma once




#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <sys/syscall.h>
#include <sys/epoll.h>
#endif

#include <string>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>
#include <unordered_map>

class Channel;


#include <vector>


struct WriteTask
{
	char* data_ = nullptr;
	size_t lens = 0;
	int sessionId = 0;
};

class EventLoop
{
public:
	EventLoop();
	~EventLoop();

public:
	void updateEvent(Channel* channel, fd_set* fds, int opt = 1);
	void startLoop();
	void processWeakUpEvent();

#ifdef _WIN32
	inline fd_set* getReads() { return &reads_; }
	inline fd_set* getWrites() { return &writes_; }
	void processReads(fd_set* fds);
	void processWrites(fd_set* fds);
#else
	void weakUp();
#endif


#ifdef _WIN32
	fd_set reads_;
	fd_set writes_;
	struct timeval timeWait_;
#else
	int efd_ = -1;
	int eventFd_ = -1;
	Channel* eventChannel_ = nullptr;
	std::vector<struct epoll_event> eventList_;
#endif
	std::mutex writeMutex_;
	std::queue<WriteTask> queue_;




};

