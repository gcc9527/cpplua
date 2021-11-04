#pragma once


#define Max_Currency_Bag_Index 200	// 最大货币背包索引,<=200的都是货币

enum enum_bag_type
{
	enum_bag_type_gold = 1, // 金币背包
	enum_bag_type_diamond = 2, // 钻石背包
	enum_bag_type_normal_item = 3, // 普通物品背包
};

enum enum_item_kind
{
	enum_item_kind_currency = 1, // 货币
	enum_item_kind_item = 2, // 普通道具
	enum_item_kind_equip = 3, // 装备
	enum_item_kind_hero = 4, // 英雄
};