

#include "Log.h"

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#else
#include <io.h>
#include <direct.h>
#include <Windows.h>
#endif



const char* arrLogType[] = { "fatal", "error", "info", "debug" };

void getLocalTime(struct tm* t, time_t* now)
{
#ifdef _WIN32
	localtime_s(t, now);
#else
	localtime_r(now, t);
#endif
}



void setColor(int logType = 4)
{
#ifdef _WIN32
	switch (logType)
	{
	case 0:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
		return;
	case 1:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED);
		return;
	case 2:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_GREEN);
		return;
	case 3:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		return;
	default:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		break;
	}
#endif
}


#define FORMATLOG(logType) \
	char buf[5000] = { 0 }; \
	struct tm tm1; \
	time_t now = time(0); \
	getLocalTime(&tm1, &now); \
	int len = sprintf(buf, "[%s][%04d-%02d-%02d %02d:%02d:%02d] ", arrLogType[logType], tm1.tm_year + 1900, tm1.tm_mon + 1, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec); \
	va_list aptr; \
	va_start(aptr, fmt); \
	vsprintf(buf + len, fmt,aptr); \
	va_end(aptr); \
	std::string s(buf); \
	setColor(logType); \
	printf("%s\n",s.c_str()); \
	setColor(4); \
	std::unique_lock<std::mutex> lk(mutex_); \
	vecLog_.push_back(std::move(s)); \
	cond_.notify_one(); \
	lk.unlock(); \




Log::Log()
{

}

Log::~Log()
{

}


void Log::init(const char* pathName, const char* logName, int level, bool isRriteFile)
{
	pathName_ = pathName;
	level_ = level;
	isRriteFile_ = isRriteFile;

#ifdef _WIN32
	if (_access(pathName, 0) != 0)
	{
		_mkdir(pathName);
	}
#else
	if (access(pathName, 0) != 0)
	{
		mkdir(pathName_, S_IRWXU);
	}
#endif
	std::string s(pathName_);
	s.append(logName);
	logName_ = logName;
	pf_ = fopen(s.c_str(), "a+");
	if (!pf_)
	{
		printf("Log::init err\n");
		std::this_thread::sleep_for(std::chrono::seconds(3));
		exit(0);
	}
	time_t now;
	time(&now);
	struct tm* tm1 = localtime(&now);
	year_ = tm1->tm_year + 1900;
	month_ = tm1->tm_mon + 1;
	day_ = tm1->tm_mday;
	hour_ = tm1->tm_hour;

	logThread_ = std::thread(&Log::writeLogThreadFunc, this);
}

void Log::writeLogThreadFunc()
{
	while (true)
	{
		vecTempLog_.clear();
		std::unique_lock<std::mutex> lk(mutex_);
		while (vecLog_.empty())
		{
			cond_.wait(lk);
		}
		vecLog_.swap(vecTempLog_);
		lk.unlock();

		for (auto& e : vecTempLog_)
		{
			e.append("\n");
			fputs(e.c_str(), pf_);
		}

		if (timeChange())
		{
			fclose(pf_);
			std::string s(pathName_);
			s.append(logName_);
			char oldName[1024] = { 0 };
			char newName[1024] = { 0 };
			sprintf(newName, "%s.%04d-%02d-%02d-%02d", s.c_str(), year_, month_, day_, hour_);
			sprintf(oldName, "%s", s.c_str());
			rename(oldName, newName);

			pf_ = fopen(s.c_str(),"a+");
		}
	}

}

bool Log::timeChange()
{

	time_t now = time(nullptr);
	struct tm tm1;
#ifdef _WIN32
	localtime_s(&tm1, &now);
#else
	localtime_r(&now, &tm1);
#endif
	if (year_ == (tm1.tm_year + 1900) &&
		month_ == tm1.tm_mon + 1 &&
		day_ == tm1.tm_mday &&
		hour_ == tm1.tm_hour)
	{
		return false;
	}
	year_ = tm1.tm_year + 1900;
	month_ = tm1.tm_mon + 1;
	day_ = tm1.tm_mday;
	hour_ = tm1.tm_hour;
	return true;
}


void Log::debug(const char* fmt, ...)
{
	if (level_ < LogLevelDebug)
	{
		return;
	}

	FORMATLOG(LogLevelDebug);


}

void Log::info(const char* fmt, ...)
{
	if (level_ < LogLevelInfo)
	{
		return;
	}

	FORMATLOG(LogLevelInfo);
}

void Log::err(const char* fmt, ...)
{
	if (level_ < LogLevelErr)
	{
		return;
	}

	FORMATLOG(LogLevelErr);
}

void Log::fatal(const char* fmt, ...)
{
	if (level_ < LogLevelFatal)
	{
		return;
	}

	FORMATLOG(LogLevelFatal);
}

