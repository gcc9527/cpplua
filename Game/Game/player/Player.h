#pragma once

#include <stdint.h>


#include "../bag/BagMgr.h"

#include "../pb/PlayerDb.pb.h"

class Player
{
public:
	Player();
	~Player();
public:
	inline int getSessionId() { return 1; }
	inline int getUid() { return 1; }
	inline std::string getName() { return ""; }
	inline std::string getIcon() { return ""; }
	inline int getVipLv() { return 0; }
	inline int getLv() { return 0; }
	void save();
	void initPlayer();
	BagMgr* getBagMgr() { return &bagMgr; }

public:
	BagMgr bagMgr;
	MsgPlayer baseData;
};

