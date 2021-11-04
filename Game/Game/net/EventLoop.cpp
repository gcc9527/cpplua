

#include "EventLoop.h"

#include <thread>
#include "Channel.h"


#ifdef __linux__
#include <sys/eventfd.h>
#include <unistd.h>
#endif

#include "../log/Log.h"
#include "Nettools.h"
#include "TcpConnecter.h"
#include "TcpServer.h"



extern std::unordered_map<int, Channel*> gChannels;
extern std::mutex gMainMutex;
extern std::unordered_map<int, TcpConnecter*> gClients;

EventLoop::EventLoop()
{
#ifdef _WIN32
	FD_ZERO(&reads_);
	FD_ZERO(&writes_);
#else
	efd_ = epoll_create(1);
	Nettools::check(efd_, "epoll_create");
	eventList_.resize(16); // 默认事件数量
	eventFd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	Nettools::check(eventFd_, "epoll_create");
#endif
}




EventLoop::~EventLoop()
{
#ifdef __linux__
	if (eventChannel_)
	{
		delete eventChannel_;
		eventChannel_ = nullptr;
	}
#endif

}



void EventLoop::updateEvent(Channel* channel, fd_set* fds, int opt)
{
#ifdef _WIN32
	if (fds == nullptr)
	{
		logDebug("EventLoop::updateEvent err, opt = %d", opt);
		return;
	}
	if (opt == 1)
	{
		FD_SET(channel->getFd(), fds);
	}
	else if (opt == 2)
	{
		for (size_t i = 0; i < fds->fd_count; ++i)
		{
			if (fds->fd_array[i] == channel->getFd())
			{
				int nextFd = 0;
				if (i + 1 < fds->fd_count)
				{
					nextFd = (int)fds->fd_array[i + 1];
				}
				fds->fd_array[i] = nextFd;
				fds->fd_count -= 1;
				break;
			}
		}
	}
#else
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.events = channel->getEvent();
	ev.data.ptr = channel;
	epoll_ctl(efd_, opt, channel->getFd(), &ev);
#endif
}

void EventLoop::startLoop()
{
#ifdef _WIN32
	fd_set tmpRead;
	fd_set tmpWrite;
#else
	eventChannel_ = new Channel(eventFd_);
	updateEvent(eventChannel_, nullptr);
#endif

	while (true)
	{

		int fds = -1;
#ifdef _WIN32
		tmpRead = reads_;
		tmpWrite = writes_;
		timeWait_.tv_sec = 5;
		timeWait_.tv_usec = 0;

		fds = select(0, &tmpRead, &tmpWrite, nullptr, &timeWait_);
#else
		fds = epoll_wait(efd_, &(*eventList_.begin()), (int)eventList_.size(), -1);
#endif
		//log("startLoop %d", std::this_thread::get_id());
		const char* errs = "err epoll_wait";
		if (fds < 0)
		{
#ifdef _WIN32
			logDebug("select err code = %d", WSAGetLastError());

#endif
			perror(errs);
		}
		else
		{

#ifdef _WIN32
			processReads(&tmpRead);
			processWrites(&tmpWrite);
#else
			for (int i = 0; i < fds; ++i)
			{
				Channel* ch = (Channel*)eventList_[i].data.ptr;
				if (ch)
				{
					if (ch->getFd() == eventFd_)
					{
						int64_t val;
						read(eventFd_, &val, 8);
					}
					else
					{
						ch->setEvent(eventList_[i].events);
						ch->onEvent();
					}
				}
			}

			if (fds == eventList_.size())
			{
				eventList_.resize(fds * 2);
			}
#endif

			processWeakUpEvent();
		}
	}
}


void EventLoop::processWeakUpEvent()
{
	WriteTask t;
	TcpConnecter* con = nullptr;

	writeMutex_.lock();
	while (!queue_.empty())
	{
		t = queue_.front();

		gMainMutex.lock();
		auto it = gClients.find(t.sessionId);
		if (it == gClients.end())
		{
			queue_.pop();
			gMainMutex.unlock();
			continue;
		}
		con = it->second;
		gMainMutex.unlock();

		if (con->isWriteing())
		{
			writeMutex_.unlock();
			return;
		}

		queue_.pop();
		break;
	}
	writeMutex_.unlock();

	if (con)
	{
		con->nowWrite(t.data_, t.lens);
		logDebug("send ok");
	}
}

#ifdef _WIN32
#define Epollin 1
#define Epollout 4
void EventLoop::processReads(fd_set* fds)
{
	int cnt = (int)fds->fd_count;
	for (int i = 0; i < cnt; ++i)
	{
		int fd = (int)fds->fd_array[i];
		if (FD_ISSET(fd, &reads_))
		{
			auto it = gChannels.find(fd);
			if (it != gChannels.end())
			{
				it->second->setEvent(Epollin);
				it->second->onEvent();
			}
			else
			{
				logDebug("processReads no find fd = %d", fd);
			}
		}
	}
}

void EventLoop::processWrites(fd_set* fds)
{
	int cnt = (int)fds->fd_count;
	for (int i = 0; i < cnt; ++i)
	{
		if (FD_ISSET(fds->fd_array[i], &writes_))
		{
			int fd = (int)fds->fd_array[i];
			auto it = gChannels.find(fd);
			if (it != gChannels.end())
			{
				it->second->setEvent(Epollout);
				it->second->onEvent();
			}
			else
			{
				logDebug("processWrites no find fd = %d", fd);
			}
		}
	}
}


#endif

#ifdef __linux__
void EventLoop::weakUp()
{
	int64_t val = 1;
	write(eventFd_, &val, 8);
}
#endif
