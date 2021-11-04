#pragma once

#include <stdint.h>

class Monitor
{

public:
	Monitor();
	~Monitor();
public:
	inline int getListenFd() { return listenFd_; }
	void setPort(uint16_t port) { port_ = port; }
private:
	int listenFd_;
	uint16_t port_ = 8888;
};

