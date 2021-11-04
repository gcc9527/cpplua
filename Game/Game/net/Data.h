#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <stdint.h>

// 客户端操作
struct ClientOpt
{
	uint8_t opt = 0; // 1:新连接 2:关闭连接
	int sessionId = 0;
};

// 发送数据包
struct Messages
{
	int who = 0; // 0 客户端 1:服务器
	char* data_ = nullptr;
	size_t lens_ = 0;
	int sessionId = 0;
};




struct Task
{
	bool second_; // 定时器任务

	ClientOpt clientOpt_;
	Messages message_;
	std::string cmd_;
	Task(bool ok = false) :
		second_(ok)
	{

	}
};




extern std::queue<Task> gQueue;
extern std::mutex gMutex;
extern std::condition_variable gCondVar;

