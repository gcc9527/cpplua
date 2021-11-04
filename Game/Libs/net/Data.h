#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>


struct Task
{
	bool ok_;
	int fd_;
	std::string str_;
	Task(bool ok = false) :
		ok_(ok),
		fd_(0)
	{

	}
};




extern std::queue<Task> gQueue;
extern std::mutex gMutex;
extern std::condition_variable gCondVar;

