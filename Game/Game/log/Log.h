
#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../public/Singleton.h"

class Log : public Singleton<Log>
{
public:
	Log();
	~Log();
public:
	enum LogLevel
	{
		LogLevelFatal = 0,
		LogLevelErr = 1,
		LogLevelInfo = 2,
		LogLevelDebug = 3,
	};

	void writeLogThreadFunc();
	void init(const char* pathName, const char* logName, int level = LogLevelDebug, bool isRriteFile = false);

	void debug(const char* fmt, ...);
	void info(const char* fmt, ...);
	void err(const char* fmt, ...);
	void fatal(const char* fmt, ...);
private:
	bool timeChange();
	const char* pathName_;
	const char* logName_;
	int level_;
	bool isRriteFile_;
	FILE* pf_;
	int year_;
	int month_;
	int day_;
	int hour_;
	std::thread logThread_;
	std::vector<std::string> vecLog_;
	std::vector<std::string> vecTempLog_;
	std::mutex mutex_;
	std::condition_variable cond_;

};

#define logInit(path, name) Log::instance().init(path, name)
#define logDebug(fmt, ...) Log::instance().debug(fmt, __VA_ARGS__)
#define logInfo(fmt, ...) Log::instance().info(fmt, __VA_ARGS__)
#define logErr(fmt, ...) Log::instance().err(fmt, __VA_ARGS__)
#define logFatal(fmt, ...) Log::instance().fatal(fmt, __VA_ARGS__)