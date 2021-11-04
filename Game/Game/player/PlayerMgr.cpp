#include "PlayerMgr.h"
#include "../log/Log.h"
#include "../MainThread.h"

PlayerMgr::PlayerMgr()
{

}

PlayerMgr::~PlayerMgr()
{

}

Player* PlayerMgr::findPlayer(int sessionId)
{
	auto it = players.find(sessionId);
	if (it == players.end())
	{
		return nullptr;
	}
	return it->second;
}

void PlayerMgr::addPlayer(int sessionId, Player* player)
{
	players.emplace(sessionId, player);
}

void PlayerMgr::save(int sessionId)
{
	//Player* player = findPlayer(sessionId);
	//if (!player)
	//{
	//	logDebug("PlayerMgr::save no find player");
	//	return;
	//}

	//DbTask t;
	//t.player_ = player;
	//std::unique_lock<std::mutex> lk(gMainThread.mutex_);
	//gMainThread.queueTask_.push(t);
	//gMainThread.cond_.notify_one();
	//lk.unlock();
}

