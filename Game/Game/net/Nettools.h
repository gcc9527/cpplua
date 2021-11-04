#pragma once

#include <stdlib.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#endif


#include <iostream>

#include "../log/Log.h"

namespace Nettools
{

	inline static void check(int val, const char* tips)
	{
		bool ok = true;
#ifdef _WIN32
		if (val != 0)
		{
			ok = false;
			logErr("WSAGetLastError = %d", WSAGetLastError);
			WSACleanup();
		}
#else
		if (val < 0)
		{
			ok = false;
		}
#endif
		if (!ok)
		{
			perror(tips);
			exit(0);
		}
	}

	inline static int setAddrReuse(int fd)
	{
		int opt = 1;
		return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof opt);
	}

#ifdef __linux__
	inline static int setNonBlock(int fd)
	{
		return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
	}
#endif


}

