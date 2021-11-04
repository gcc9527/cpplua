



#include "Monitor.h"
#include "Nettools.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#include<ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#endif

#include <iostream>

#include "../log/Log.h"

Monitor::Monitor()
{

#ifdef _WIN32
	WSADATA wsa;
	Nettools::check(WSAStartup(MAKEWORD(2, 2), &wsa), "WSAStartup");
#endif

	listenFd_ = (int)socket(AF_INET, SOCK_STREAM, 0);

#ifdef _WIN32
	if (listenFd_ == INVALID_SOCKET)
	{
		logErr("socket err = %d", WSAGetLastError());
		WSACleanup();
		exit(0);
	}
	u_long iMode = 0;
	Nettools::check(ioctlsocket(listenFd_, FIONBIO, &iMode), "ioctlsocket");
#else
	Nettools::check(listenFd_, "socket");
	Nettools::check(Nettools::setNonBlock(listenFd_), "setNonBlock");
#endif

	Nettools::check(Nettools::setAddrReuse(listenFd_), "setAddrReuse");

	struct sockaddr_in addr;
	addr.sin_port = htons(port_);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	socklen_t len = sizeof addr;
	int ret = bind(listenFd_, (struct sockaddr*)&addr, len);
	Nettools::check(ret, "bind");
	Nettools::check(listen(listenFd_, SOMAXCONN), "listen");
}

Monitor::~Monitor()
{

}
