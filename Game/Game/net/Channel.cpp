#include "Channel.h"

#include <stdio.h>




Channel::Channel(int fd) :
	fd_(fd),
	event_(1),
	revent_(0),
	isClose_(false),
	con_(nullptr)
{

}

Channel::~Channel()
{
	con_ = nullptr;
}

#define Epollin 1
#define Epollout 4


void Channel::onEvent()
{
	if ((revent_ & Epollin) && readCb_)
	{
		readCb_();
	}

	if ((revent_ & Epollout) && writeCb_)
	{
		writeCb_();
	}

	if (isClose_ && (revent_ & Epollin) && readCb_)
	{
		readCb_();
	}
}
