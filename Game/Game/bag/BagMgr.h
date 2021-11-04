#pragma once

#include <stdint.h>

#include <map>


class Player;

class BagMgr 
{
public:
	BagMgr();
	~BagMgr();

	void initBag(Player* player);
	bool isFullById(int itemId);
	bool isFullByUid(int uid);
	std::map<int, int64_t> addItem(int itemId, int64_t itemCnt, int reason = 0);
	void addItemList(const std::map<int, int64_t>& itemList, int reason = 0);
	void removeItemByUid(int uid, int64_t itemCnt, int reason = 0);
	std::map<int, int64_t> removeItemById(int itemId, int64_t itemCnt, int reason = 0);
	void removeItemListByUid(const std::map<int, int64_t>& itemList, int reason = 0);
	bool removeItemListById(const std::map<int, int64_t>& itemList, int reason = 0);
	int64_t getItemCntById(int itemId);
	int64_t getItemCntByUid(int uid);
	bool isEnoughById(int itemId, int64_t itemCnt);
	bool isEnoughByUid(int uid);
	bool isEnoughListById(const std::map<int, int64_t>& itemList);
	bool isEnoughListByUId(const std::map<int, int64_t>& itemList);
private:
	Player* player = nullptr;
};


