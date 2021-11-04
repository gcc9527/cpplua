#pragma once

#include <stdint.h>

#include <unordered_map>


#include "../public/Singleton.h"

class Player;

class PlayerMgr : public Singleton<PlayerMgr>
{
public:
	PlayerMgr();
	~PlayerMgr();
	

	bool isInCache();
	Player* findPlayer(int sessionId);
	void addPlayer(int sessionId, Player* player);
	void save(int sessionId);
private:
	std::unordered_map<int, Player*> players;
};

#define gPlayerMgr PlayerMgr::instance()