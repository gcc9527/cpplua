
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include "../log/Log.h"
#include "../json/json.h"
#include "../public/Singleton.h"
#include "../public/ExitThread.hpp"
#include "../define/Define.h"




/**
 * 配置表数据管理
 */

// 物品表数据结构
struct ConfItem
{
	int id; // 物品id
	const char* name; // 物品名字
	bool pile; // 是否可以堆叠
	short kind; // 物品类型
	short index; // 背包索引

	bool isCurrency() const
	{
		return kind == 1;
	}
};

// 背包数据结构
struct ConfBagType
{
	short index; // 背包索引
	short bagSize; // 背包默认大小
	char exMaxCnt; // 扩容上限
	std::map<int, int> consume; // 扩容消耗
};

struct ConfItemMgr :public Singleton<ConfItemMgr>
{

	const ConfItem* getItem(int itemId)
	{
		auto it = datas.find(itemId);
		if (it != datas.end())
		{
			return &it->second;
		}
		return nullptr;
	}
	std::unordered_map<int, ConfItem> datas;
};

struct ConfBagTypeMgr :public Singleton<ConfBagTypeMgr>
{
	std::unordered_map<int, ConfBagType> datas;
};






#define gConfBagTypeMgr ConfBagTypeMgr::instance()
#define gConfItemMgr ConfItemMgr::instance()































inline bool parseFile(const char* func, const char* confPath, Json::Value& root)
{
	std::ifstream ifs;
	ifs.open(confPath);
	if (!ifs.is_open())
	{
		logDebug("%s parseFile !ifs.is_open", func);
		return false;
	}
	Json::CharReaderBuilder builder;
	//std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
	JSONCPP_STRING err;

	if (!Json::parseFromStream(builder, ifs, &root, &err))
	{
		logDebug("%s parseItem !reader.parse, err = %s", func, err.c_str());
		return false;
	}

	return true;
}

inline bool getBoolVal(int val)
{
	if (val)
	{
		return true;
	}

	return false;
}

inline bool initItem(const char* confPath)
{
	Json::Value root;
	if (!parseFile("parseItem", confPath, root))
		return false;

	for (auto& e : root.getMemberNames())
	{
		ConfItem item;
		Json::Value& val = root[e];
		item.id = val["id"].asInt();
		item.name = val["name"].asCString();
		item.pile = getBoolVal(val["pile"].asInt());
		item.kind = val["kind"].asInt();
		if (item.kind == 1 && (!item.pile))
		{
			logErr("initItem item.kind == 1 && (!item.pile)");
			ExitPro::exitProcess();
		}
		item.index = val["index"].asInt();
		gConfItemMgr.instance().datas.emplace(item.id, std::move(item));
	}

	return true;
}

inline bool initBagType(const char* confPath)
{
	Json::Value root;
	if (!parseFile("parseBagType", confPath, root))
		return false;


	for (auto& e : root.getMemberNames())
	{
		ConfBagType item;
		Json::Value& val = root[e];
		item.index = val["index"].asInt();
		item.bagSize = val["bagSize"].asInt();

		if (item.index >= Max_Currency_Bag_Index && item.bagSize <= 0)
		{
			logErr("initBagType item.index >= Max_Currency_Bag_Index && item.bagSize <= 0");
			ExitPro::exitProcess();
		}

		item.exMaxCnt = val["exMaxCnt"].asInt();

		Json::Value& consume = val["consume"];
		for (auto& ee : consume)
		{
			item.consume.emplace(ee["id"].asInt(), ee["cnt"].asInt());
		}

		gConfBagTypeMgr.instance().datas.emplace(item.index, std::move(item));
	}

	return true;
}


inline bool initConfig()
{
	if (!initItem("../Config/item.json"))
		return false;

	if (!initBagType("../Config/bagType.json"))
		return false;

	return true;
}







