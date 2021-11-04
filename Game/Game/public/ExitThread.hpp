#pragma once


#include <thread>


namespace ExitPro
{
	inline static void exitProcess()
	{
		std::this_thread::sleep_for(std::chrono::seconds(5));
		exit(0);
	}

}