#include "BagMgr.h"

#include <stack>
#include <vector>
#include <map>
#include <math.h>

#include "../cfg/Cfg.h"
#include "../player/Player.h"
#include "../pb/PlayerDb.pb.h"
#include "../log/Log.h"
#include "../public/Lua.hpp"


std::stack<MsgPlayerBagItem> gStack;
int gItemUid = 1;



enum enum_hero_part
{
	enum_hero_part_weap = 1, // 武器
	enum_hero_part_cuirass = 2, // 胸甲
	enum_hero_part_shoes = 3, // 鞋子
	enum_hero_part_helmet = 4, // 头盔
	enum_hero_part_rune = 5, // 符文
	enum_hero_part_gem = 6, // 宝石

	enum_hero_part_max,	// 结束标志
};






BagMgr::BagMgr()
{

}

BagMgr::~BagMgr()
{

}

void BagMgr::initBag(Player* player)
{
	this->player = player;

	auto bags = player->baseData.mutable_msgplayerbag();
	for (auto& e : gConfBagTypeMgr.datas)
	{
		if (bags->find(e.first) == bags->end())
		{
			MsgPlayerBag bag;
			bag.set_cap(e.second.bagSize);
			bags->insert({ e.first, std::move(bag) });
		}
	}
}

bool BagMgr::isFullById(int itemId)
{
	auto confItem = gConfItemMgr.getItem(itemId);
	if (!confItem)
	{
		logErr("BagMgr::bagIsFullById conf no find item, itemid = %d", itemId);
		return true;
	}

	auto bags = player->baseData.mutable_msgplayerbag();

	if (bags->find(confItem->index) == bags->end())
	{
		logErr("BagMgr::bagIsFullById no this idx, bagidx = %d, itemid = %d", confItem->index, itemId);
		return true;
	}

	MsgPlayerBag& bag = bags->operator[](confItem->index);

	if (confItem->pile)
	{
		return false;
	}

	return bag.cap() >= bag.msgplayerbagitem().size();

}



bool BagMgr::isFullByUid(int uid)
{
	return true;
}

std::map<int, int64_t> BagMgr::addItem(int itemId, int64_t itemCnt, int reason /*= 0*/)
{
	if (itemCnt <= 0)
	{
		logErr("BagMgr::addItem itemCnt is 0, itemid = %d, itemCnt=%lld", itemId, itemCnt);
		return {};
	}
	auto confItem = gConfItemMgr.getItem(itemId);
	if (!confItem)
	{
		logErr("BagMgr::addItem conf no find confitem, itemid = %d", itemId);
		return {};
	}

	std::map<int, int64_t> changeList;

	auto bags = player->baseData.mutable_msgplayerbag();
	auto it = bags->find(confItem->index);


	if (it != bags->end())
	{
		MsgPlayerBag& bag = it->second;
		auto items = bag.mutable_msgplayerbagitem();
		bool ok = false;

		if (confItem->pile)
		{
			auto e = items->begin();
			for (; e != items->end(); ++e)
			{
				if (e->second.id() == itemId)
				{
					break;
				}
			}

			if (e != items->end())
			{
				e->second.set_cnt(e->second.cnt() + itemCnt);
				changeList.emplace(e->second.uid(), e->second.cnt());

			}
			else
			{
				MsgPlayerBagItem item;
				item.set_uid(gItemUid++);
				item.set_id(itemId);
				item.set_cnt(itemCnt);
				items->insert({ item.uid(), item });
				changeList.emplace(item.uid(), itemCnt);
			}
		}
		else
		{
			int cnt = (int)items->size();
			int leftCnt = bag.cap() - cnt;
			if (leftCnt <= 0)
			{
				// 发邮件
				return {};
			}

			int uid = 0;
			if (!gStack.empty())
			{
				MsgPlayerBagItem item = gStack.top();
				gStack.pop();

				item.set_id(itemId);
				item.set_cnt(itemCnt);
				item.clear_attr();
				item.clear_dynaattr();
				item.clear_skills();
				items->insert({ item.uid(), item });
				changeList.emplace(item.uid(), itemCnt);
				uid = item.uid();
			}
			else
			{
				MsgPlayerBagItem item;
				item.set_uid(gItemUid++);
				item.set_id(itemId);
				item.set_cnt(itemCnt);
				items->insert({ item.uid(), item });
				changeList.emplace(item.uid(), itemCnt);
				uid = item.uid();

			}

			if (confItem->kind == enum_item_kind_hero && (uid != 0))
			{
				auto* msgHero = player->baseData.mutable_msghero();
				MsgHero hero;
				hero.set_id(itemId);
				hero.set_lv(1);
				hero.set_uid(uid);
				LuaBind::CallLua cl("getHeroAttr", "HeroMgr");
				std::map<int, int> attr = cl.call<std::map<int, int>, void>(itemId);
				auto* attrs = hero.mutable_attrs();
				for (auto& e : attr)
				{
					attrs->insert({ e.first, e.second });
				}
				msgHero->insert({ uid, hero });
			}
		}


	}
	else
	{
		logErr("BagMgr::addItem no this idx, itemid = %d, idx = %d", itemId, confItem->index);
	}

	return changeList;
}

void BagMgr::addItemList(const std::map<int, int64_t>& itemList, int reason /*= 0*/)
{
	for (auto& e : itemList)
	{
		addItem(e.first, e.second, reason);
	}
}

void BagMgr::removeItemByUid(int uid, int64_t itemCnt, int reason /*= 0*/)
{

}

std::map<int, int64_t> BagMgr::removeItemById(int itemId, int64_t itemCnt, int reason /*= 0*/)
{
	if (itemCnt <= 0)
	{
		logErr("BagMgr::removeItemById conf no find item, itemid = %d, cnt = %lld", itemId, itemCnt);
		return {};
	}
	auto confItem = gConfItemMgr.getItem(itemId);
	if (!confItem)
	{
		logErr("BagMgr::removeItemById conf no find confItem, itemid = %d", itemId);
		return {};
	}

	std::map<int, int64_t> changeList;

	auto bags = player->baseData.mutable_msgplayerbag();
	auto it = bags->find(confItem->index);
	if (it != bags->end())
	{
		MsgPlayerBag& bag = bags->operator[](confItem->index);
		auto items = bag.mutable_msgplayerbagitem();

		for (auto e = items->begin(); e != items->end(); ++e)
		{
			if (e->second.id() == itemId)
			{
				int64_t curCnt = e->second.cnt();
				curCnt -= itemCnt;

				if (curCnt <= 0)
				{
					e->second.set_cnt(0);
				}
				else
				{
					e->second.set_cnt(curCnt);
				}
				changeList.emplace(e->first, e->second.cnt());
			}
		}
		if (changeList.size() == 0)
		{
			logErr("BagMgr::removeItemById no find item, itemid = %d, idx = %d", itemId, confItem->index);
		}
	}
	else
	{
		logErr("BagMgr::removeItemById no this idx, itemid = %d, idx = %d", itemId, confItem->index);
	}

	return changeList;
}

void BagMgr::removeItemListByUid(const std::map<int, int64_t>& itemList, int reason /*= 0*/)
{

}

bool BagMgr::removeItemListById(const std::map<int, int64_t>& itemList, int reason /*= 0*/)
{
	for (auto& e : itemList)
	{
		removeItemById(e.first, e.second, reason);

	}

	return true;
}

int64_t BagMgr::getItemCntById(int itemId)
{
	auto confItem = gConfItemMgr.getItem(itemId);
	if (!confItem)
	{
		logErr("BagMgr::isEnoughById conf no find item, itemid = %d", itemId);
		return 0;
	}

	auto bags = player->baseData.mutable_msgplayerbag();
	auto it = bags->find(confItem->index);
	if (it != bags->end())
	{
		MsgPlayerBag& bag = bags->operator[](confItem->index);
		auto items = bag.mutable_msgplayerbagitem();
		auto e = items->begin();
		for (; e != items->end(); ++it)
		{
			if (e->second.id() == itemId)
			{
				return e->second.cnt();
			}
		}
	}
	else
	{
		logErr("BagMgr::addItem no this idx, itemid = %d, idx = %d", itemId, confItem->index);
	}

	return 0;
}

int64_t BagMgr::getItemCntByUid(int uid)
{
	return 0;
}

bool BagMgr::isEnoughById(int itemId, int64_t itemCnt)
{
	auto confItem = gConfItemMgr.getItem(itemId);
	if (!confItem)
	{
		logErr("BagMgr::isEnoughById conf no find item, itemid = %d", itemId);
		return false;
	}

	if (confItem->pile)
	{
		return true;
	}


	auto bags = player->baseData.mutable_msgplayerbag();
	auto it = bags->find(confItem->index);
	if (it != bags->end())
	{
		MsgPlayerBag& bag = bags->operator[](confItem->index);
		return bag.cap() >= bag.msgplayerbagitem().size();
	}
	else
	{
		logErr("BagMgr::isEnoughById no this idx, itemid = %d, idx = %d", itemId, confItem->index);
	}

	return false;
}

bool BagMgr::isEnoughByUid(int uid)

{
	return 0;
}

bool BagMgr::isEnoughListById(const std::map<int, int64_t>& itemList)
{
	for (auto& e : itemList)
	{
		if (!BagMgr::isEnoughById(e.first, e.second))
			return false;
	}

	return true;
}

bool BagMgr::isEnoughListByUId(const std::map<int, int64_t>& itemList)
{
	return false;
}
