
#pragma once

#include <stdio.h>
#include <vector>
#include <map>
#include <string>

#include "../../Libs/lua/lua.hpp"
#include "../codec/Codec.h"
#include "../log/Log.h"





/**
 * 
 *								调用c++成员函数时,特别注意函数的参数是引用时,调用会无效.同时会错误提示出调用函数的名字
 *								如果一定要是引用参数,请加上const 如void f1(const std::string& str)
 *								
 *								
 *								如果需要引用,也请全特化const引用类型 如
 *								templte<>							  templte<>
 * 								void getLuaVal(cosnt int& a)		  int getLuaVal(const double& a)
 * 
 *                              
 *                              不支持非const类型的引用,请特别注意
 * 
 * 
 * 
 *                              特别注意:注册给lua的c++对象类型,非基础类型,统一使用非const的指针形式, 如 xxx* x,不支持const类型.值类型
 *							    引用类型
 * 
 * 
 * 
 */


extern lua_State* gLuaSgate;

struct LuaMsg
{
	char* data_ = nullptr;
	size_t lens_ = 0;
	const char* name_;
	bool ok_ = false;
};


namespace LuaBind
{
	inline void show()
	{
		int i;	int top = lua_gettop(gLuaSgate);
		printf("stackDump(num=%d):\n", top);
		for (i = 1; i <= top; i++)
		{
			int t = lua_type(gLuaSgate, i);
			switch (t)
			{
			case LUA_TSTRING:
				printf("`%s'", lua_tostring(gLuaSgate, i));
				break;
			case LUA_TBOOLEAN:
				printf(lua_toboolean(gLuaSgate, i) ? "true" : "false");
			break;		case LUA_TNUMBER:
				printf("%g", lua_tonumber(gLuaSgate, i));
				break;
			default:
				printf("%s", lua_typename(gLuaSgate, t));
				break;
			}
			printf("  ");
		}
		printf("\n");

	}

	struct BaseFunc
	{
		BaseFunc() {}
		~BaseFunc(){}
		virtual int call(lua_State* L) { return 0; }
	};


	inline const char* errName(const char* val)
	{
		if (!val)
		{
			return "unknown";
		}
		return val;
	}

	inline void showStack(int deep)
	{
		lua_Debug debug;
		if (lua_getstack(gLuaSgate, deep, &debug))
		{
			lua_getinfo(gLuaSgate, "Sln", &debug);
			if (debug.name)
			{
				logDebug("line = %d, errName = %s, errMess = %s : line = %d", debug.currentline, errName(debug.name), debug.short_src, debug.linedefined);
			}
			showStack(deep + 1);
		}
	}

	inline int errFunc(lua_State* L)
	{
		logDebug("error : %s", lua_tostring(gLuaSgate, -1));
		showStack(0);
		return 0;
	}

	template<typename T>
	struct GlobalName
	{
		static const char* clsName;
		static inline void setName(const char* name) { clsName = name; }
		static inline const char* getName() { return clsName; }
	};

	template<typename T>
	const char* GlobalName<T>::clsName = nullptr;

	template<typename T>
	struct ObjData
	{
		ObjData(T* obj) { this->obj = obj; }
		~ObjData() { obj = nullptr; }
		T* obj;
	};

	template<typename T>
	struct GetCppObj
	{
		inline static T getCppObj(int idx)
		{
			ObjData<T>** od = (ObjData<T>**)lua_touserdata(gLuaSgate, idx);
			return *((*od)->obj);
		}
	};

	template<typename T>
	struct GetCppObj<T&>
	{
		inline static T& getCppObj(int idx)
		{
			ObjData<T>** od = (ObjData<T>**)(lua_touserdata(gLuaSgate, idx));
			return *((*od)->obj);
		}
	};


	template<typename T>
	struct GetCppObj<T*>
	{
		inline static T* getCppObj(int idx)
		{
			ObjData<T>** od = (ObjData<T>**)(lua_touserdata(gLuaSgate, idx));
			return (*od)->obj;
		}
	};

	template<typename T>
	inline T getLuaVal(int idx)
	{
		if (!lua_isuserdata(gLuaSgate, idx))
		{
			lua_pushfstring(gLuaSgate, "%d pos agr no userdata", idx);
			lua_error(gLuaSgate);
		}
		return GetCppObj<T>::getCppObj(idx);
	}

	template<typename T>
	inline std::vector<T> getVevtor(int idx)
	{
		std::vector<T> vec;
		int cnt = (int)luaL_len(gLuaSgate, idx);
		for (int i = 1; i <= cnt; ++i)
		{
			lua_rawgeti(gLuaSgate, idx, i);
			T t = getLuaVal<T>(-1);
			vec.push_back(t);
		}
		return vec;
	}

	template<typename K, typename V>
	inline std::map<K, V> getMapVal(int idx)
	{
		lua_pushnil(gLuaSgate);
		std::map<K, V> tab;
		while (lua_next(gLuaSgate, idx) != 0)
		{ //-1=val, -2=k
			lua_pushvalue(gLuaSgate, -2);
			K k = getLuaVal<K>(-1);
			V v = getLuaVal<V>(-2);
			tab.insert({ k,v });
			lua_pop(gLuaSgate, 2);
		}
		return tab;
	}

	template<> inline std::string getLuaVal(int idx) { size_t len; const char* s = (const char*)lua_tolstring(gLuaSgate, idx, &len); return std::string(s, len); }
	template<> inline const std::string& getLuaVal(int idx) { size_t len; const char* s = (const char*)lua_tolstring(gLuaSgate, idx, &len); return std::string(s, len); }
	template<> inline std::string&& getLuaVal(int idx) { size_t len; const char* s = (const char*)lua_tolstring(gLuaSgate, idx, &len); return std::move(std::string(s, len)); }
	template<> inline bool getLuaVal(int idx){ return lua_toboolean(gLuaSgate, idx) ? true : false; }
	template<> inline char* getLuaVal(int idx) { return (char*)lua_tostring(gLuaSgate, idx); }
	template<> inline const char* getLuaVal(int idx) { return (const char*)lua_tostring(gLuaSgate, idx); }
	template<> inline char getLuaVal(int idx) { return (char)lua_tonumber(gLuaSgate, idx); }
	template<> inline unsigned char getLuaVal(int idx) { return (unsigned char)lua_tonumber(gLuaSgate, idx); }
	template<> inline short getLuaVal(int idx) { return (short)lua_tonumber(gLuaSgate, idx); }
	template<> inline unsigned short getLuaVal(int idx) { return (unsigned short)lua_tonumber(gLuaSgate, idx); }
	template<> inline long getLuaVal(int idx) { return (long)lua_tonumber(gLuaSgate, idx); }
	template<> inline unsigned long getLuaVal(int idx) { return (unsigned long)lua_tonumber(gLuaSgate, idx); }
	template<> inline int getLuaVal(int idx) { return (int)lua_tonumber(gLuaSgate, idx); }
	template<> inline unsigned int getLuaVal(int idx) { return (unsigned int)lua_tonumber(gLuaSgate, idx); }
	template<> inline long long getLuaVal(int idx) { return (long long)lua_tonumber(gLuaSgate, idx); }
	template<> inline unsigned long long getLuaVal(int idx) { return (unsigned long long)lua_tonumber(gLuaSgate, idx); }
	template<> inline float getLuaVal(int idx) { return (float)lua_tonumber(gLuaSgate, idx); }
	template<> inline double getLuaVal(int idx) { return (double)lua_tonumber(gLuaSgate, idx); }
	template<> inline std::vector<bool> getLuaVal(int idx) { return getVevtor<bool>(idx); }
	template<> inline std::vector<char> getLuaVal(int idx) { return getVevtor<char>(idx); }
	template<> inline std::vector<char*> getLuaVal(int idx) { return getVevtor<char*>(idx); }
	template<> inline std::vector<const char*> getLuaVal(int idx) { return getVevtor<const char*>(idx); }
	template<> inline std::vector<unsigned char> getLuaVal(int idx) { return getVevtor<unsigned char>(idx); }
	template<> inline std::vector<short> getLuaVal(int idx) { return getVevtor<short>(idx); }
	template<> inline std::vector<unsigned short> getLuaVal(int idx) { return getVevtor<unsigned short>(idx); }
	template<> inline std::vector<float> getLuaVal(int idx) { return getVevtor<float>(idx); }
	template<> inline std::vector<double> getLuaVal(int idx) { return getVevtor<double>(idx); }
	template<> inline std::vector<int> getLuaVal(int idx) { return getVevtor<int>(idx); }
	template<> inline std::vector<unsigned int> getLuaVal(int idx) { return getVevtor<unsigned int>(idx); }
	template<> inline std::vector<long> getLuaVal(int idx) { return getVevtor<long>(idx); }
	template<> inline std::vector<unsigned long> getLuaVal(int idx) { return getVevtor<unsigned long>(idx); }
	template<> inline std::vector<long long> getLuaVal(int idx) { return getVevtor<long long>(idx); }
	template<> inline std::vector<unsigned long long> getLuaVal(int idx) { return getVevtor<unsigned long long>(idx); }
	template<> inline std::map<bool, bool> getLuaVal(int idx) { return getMapVal<bool, bool>(idx); }
	template<> inline std::map<bool, char> getLuaVal(int idx) { return getMapVal<bool, char>(idx); }
	template<> inline std::map<bool, char*> getLuaVal(int idx) { return getMapVal<bool, char*>(idx); }
	template<> inline std::map<bool, const char*> getLuaVal(int idx) { return getMapVal<bool, const char*>(idx); }
	template<> inline std::map<bool, float> getLuaVal(int idx) { return getMapVal<bool, float>(idx); }
	template<> inline std::map<bool, double> getLuaVal(int idx) { return getMapVal<bool, double>(idx); }
	template<> inline std::map<bool, short> getLuaVal(int idx) { return getMapVal<bool, short>(idx); }
	template<> inline std::map<bool, int> getLuaVal(int idx) { return getMapVal<bool, int>(idx); }
	template<> inline std::map<bool, long> getLuaVal(int idx) { return getMapVal<bool, long>(idx); }
	template<> inline std::map<bool, long long> getLuaVal(int idx) { return getMapVal<bool, long long>(idx); }
	template<> inline std::map<bool, unsigned char> getLuaVal(int idx) { return getMapVal<bool, unsigned char>(idx); }
	template<> inline std::map<bool, unsigned short> getLuaVal(int idx) { return getMapVal<bool, unsigned short>(idx); }
	template<> inline std::map<bool, unsigned int> getLuaVal(int idx) { return getMapVal<bool, unsigned int>(idx); }
	template<> inline std::map<bool, unsigned long> getLuaVal(int idx) { return getMapVal<bool, unsigned long>(idx); }
	template<> inline std::map<bool, unsigned long long> getLuaVal(int idx) { return getMapVal<bool, unsigned long long>(idx); }
	template<> inline std::map<char, char> getLuaVal(int idx) { return getMapVal<char, char>(idx); }
	template<> inline std::map<char, char*> getLuaVal(int idx) { return getMapVal<char, char*>(idx); }
	template<> inline std::map<char, const char*> getLuaVal(int idx) { return getMapVal<char, const char*>(idx); }
	template<> inline std::map<char, float> getLuaVal(int idx) { return getMapVal<char, float>(idx); }
	template<> inline std::map<char, double> getLuaVal(int idx) { return getMapVal<char, double>(idx); }
	template<> inline std::map<char, short> getLuaVal(int idx) { return getMapVal<char, short>(idx); }
	template<> inline std::map<char, int> getLuaVal(int idx) { return getMapVal<char, int>(idx); }
	template<> inline std::map<char, long> getLuaVal(int idx) { return getMapVal<char, long>(idx); }
	template<> inline std::map<char, long long> getLuaVal(int idx) { return getMapVal<char, long long>(idx); }
	template<> inline std::map<char, unsigned char> getLuaVal(int idx) { return getMapVal<char, unsigned char>(idx); }
	template<> inline std::map<char, unsigned short> getLuaVal(int idx) { return getMapVal<char, unsigned short>(idx); }
	template<> inline std::map<char, unsigned int> getLuaVal(int idx) { return getMapVal<char, unsigned int>(idx); }
	template<> inline std::map<char, unsigned long> getLuaVal(int idx) { return getMapVal<char, unsigned long>(idx); }
	template<> inline std::map<char, unsigned long long> getLuaVal(int idx) { return getMapVal<char, unsigned long long>(idx); }
	template<> inline std::map<char*, char> getLuaVal(int idx) { return getMapVal<char*, char>(idx); }
	template<> inline std::map<char*, char*> getLuaVal(int idx) { return getMapVal<char*, char*>(idx); }
	template<> inline std::map<char*, const char*> getLuaVal(int idx) { return getMapVal<char*, const char*>(idx); }
	template<> inline std::map<char*, float> getLuaVal(int idx) { return getMapVal<char*, float>(idx); }
	template<> inline std::map<char*, double> getLuaVal(int idx) { return getMapVal<char*, double>(idx); }
	template<> inline std::map<char*, short> getLuaVal(int idx) { return getMapVal<char*, short>(idx); }
	template<> inline std::map<char*, int> getLuaVal(int idx) { return getMapVal<char*, int>(idx); }
	template<> inline std::map<char*, long> getLuaVal(int idx) { return getMapVal<char*, long>(idx); }
	template<> inline std::map<char*, long long> getLuaVal(int idx) { return getMapVal<char*, long long>(idx); }
	template<> inline std::map<char*, unsigned char> getLuaVal(int idx) { return getMapVal<char*, unsigned char>(idx); }
	template<> inline std::map<char*, unsigned short> getLuaVal(int idx) { return getMapVal<char*, unsigned short>(idx); }
	template<> inline std::map<char*, unsigned int> getLuaVal(int idx) { return getMapVal<char*, unsigned int>(idx); }
	template<> inline std::map<char*, unsigned long> getLuaVal(int idx) { return getMapVal<char*, unsigned long>(idx); }
	template<> inline std::map<char*, unsigned long long> getLuaVal(int idx) { return getMapVal<char*, unsigned long long>(idx); }
	template<> inline std::map<const char*, char> getLuaVal(int idx) { return getMapVal<const char*, char>(idx); }
	template<> inline std::map<const char*, char*> getLuaVal(int idx) { return getMapVal<const char*, char*>(idx); }
	template<> inline std::map<const char*, const char*> getLuaVal(int idx) { return getMapVal<const char*, const char*>(idx); }
	template<> inline std::map<const char*, float> getLuaVal(int idx) { return getMapVal<const char*, float>(idx); }
	template<> inline std::map<const char*, double> getLuaVal(int idx) { return getMapVal<const char*, double>(idx); }
	template<> inline std::map<const char*, short> getLuaVal(int idx) { return getMapVal<const char*, short>(idx); }
	template<> inline std::map<const char*, int> getLuaVal(int idx) { return getMapVal<const char*, int>(idx); }
	template<> inline std::map<const char*, long> getLuaVal(int idx) { return getMapVal<const char*, long>(idx); }
	template<> inline std::map<const char*, long long> getLuaVal(int idx) { return getMapVal<const char*, long long>(idx); }
	template<> inline std::map<const char*, unsigned char> getLuaVal(int idx) { return getMapVal<const char*, unsigned char>(idx); }
	template<> inline std::map<const char*, unsigned short> getLuaVal(int idx) { return getMapVal<const char*, unsigned short>(idx); }
	template<> inline std::map<const char*, unsigned int> getLuaVal(int idx) { return getMapVal<const char*, unsigned int>(idx); }
	template<> inline std::map<const char*, unsigned long> getLuaVal(int idx) { return getMapVal<const char*, unsigned long>(idx); }
	template<> inline std::map<const char*, unsigned long long> getLuaVal(int idx) { return getMapVal<const char*, unsigned long long>(idx); }
	template<> inline std::map<unsigned char, char> getLuaVal(int idx) { return getMapVal<unsigned char, char>(idx); }
	template<> inline std::map<unsigned char, char*> getLuaVal(int idx) { return getMapVal<unsigned char, char*>(idx); }
	template<> inline std::map<unsigned char, const char*> getLuaVal(int idx) { return getMapVal<unsigned char, const char*>(idx); }
	template<> inline std::map<unsigned char, float> getLuaVal(int idx) { return getMapVal<unsigned char, float>(idx); }
	template<> inline std::map<unsigned char, double> getLuaVal(int idx) { return getMapVal<unsigned char, double>(idx); }
	template<> inline std::map<unsigned char, short> getLuaVal(int idx) { return getMapVal<unsigned char, short>(idx); }
	template<> inline std::map<unsigned char, int> getLuaVal(int idx) { return getMapVal<unsigned char, int>(idx); }
	template<> inline std::map<unsigned char, long> getLuaVal(int idx) { return getMapVal<unsigned char, long>(idx); }
	template<> inline std::map<unsigned char, long long> getLuaVal(int idx) { return getMapVal<unsigned char, long long>(idx); }
	template<> inline std::map<unsigned char, unsigned char> getLuaVal(int idx) { return getMapVal<unsigned char, unsigned char>(idx); }
	template<> inline std::map<unsigned char, unsigned short> getLuaVal(int idx) { return getMapVal<unsigned char, unsigned short>(idx); }
	template<> inline std::map<unsigned char, unsigned int> getLuaVal(int idx) { return getMapVal<unsigned char, unsigned int>(idx); }
	template<> inline std::map<unsigned char, unsigned long> getLuaVal(int idx) { return getMapVal<unsigned char, unsigned long>(idx); }
	template<> inline std::map<unsigned char, unsigned long long> getLuaVal(int idx) { return getMapVal<unsigned char, unsigned long long>(idx); }
	template<> inline std::map<float, char> getLuaVal(int idx) { return getMapVal<float, char>(idx); }
	template<> inline std::map<float, char*> getLuaVal(int idx) { return getMapVal<float, char*>(idx); }
	template<> inline std::map<float, const char*> getLuaVal(int idx) { return getMapVal<float, const char*>(idx); }
	template<> inline std::map<float, float> getLuaVal(int idx) { return getMapVal<float, float>(idx); }
	template<> inline std::map<float, double> getLuaVal(int idx) { return getMapVal<float, double>(idx); }
	template<> inline std::map<float, short> getLuaVal(int idx) { return getMapVal<float, short>(idx); }
	template<> inline std::map<float, int> getLuaVal(int idx) { return getMapVal<float, int>(idx); }
	template<> inline std::map<float, long> getLuaVal(int idx) { return getMapVal<float, long>(idx); }
	template<> inline std::map<float, long long> getLuaVal(int idx) { return getMapVal<float, long long>(idx); }
	template<> inline std::map<float, unsigned char> getLuaVal(int idx) { return getMapVal<float, unsigned char>(idx); }
	template<> inline std::map<float, unsigned short> getLuaVal(int idx) { return getMapVal<float, unsigned short>(idx); }
	template<> inline std::map<float, unsigned int> getLuaVal(int idx) { return getMapVal<float, unsigned int>(idx); }
	template<> inline std::map<float, unsigned long> getLuaVal(int idx) { return getMapVal<float, unsigned long>(idx); }
	template<> inline std::map<float, unsigned long long> getLuaVal(int idx) { return getMapVal<float, unsigned long long>(idx); }
	template<> inline std::map<double, char> getLuaVal(int idx) { return getMapVal<double, char>(idx); }
	template<> inline std::map<double, char*> getLuaVal(int idx) { return getMapVal<double, char*>(idx); }
	template<> inline std::map<double, const char*> getLuaVal(int idx) { return getMapVal<double, const char*>(idx); }
	template<> inline std::map<double, float> getLuaVal(int idx) { return getMapVal<double, float>(idx); }
	template<> inline std::map<double, double> getLuaVal(int idx) { return getMapVal<double, double>(idx); }
	template<> inline std::map<double, short> getLuaVal(int idx) { return getMapVal<double, short>(idx); }
	template<> inline std::map<double, int> getLuaVal(int idx) { return getMapVal<double, int>(idx); }
	template<> inline std::map<double, long> getLuaVal(int idx) { return getMapVal<double, long>(idx); }
	template<> inline std::map<double, long long> getLuaVal(int idx) { return getMapVal<double, long long>(idx); }
	template<> inline std::map<double, unsigned char> getLuaVal(int idx) { return getMapVal<double, unsigned char>(idx); }
	template<> inline std::map<double, unsigned short> getLuaVal(int idx) { return getMapVal<double, unsigned short>(idx); }
	template<> inline std::map<double, unsigned int> getLuaVal(int idx) { return getMapVal<double, unsigned int>(idx); }
	template<> inline std::map<double, unsigned long> getLuaVal(int idx) { return getMapVal<double, unsigned long>(idx); }
	template<> inline std::map<double, unsigned long long> getLuaVal(int idx) { return getMapVal<double, unsigned long long>(idx); }
	template<> inline std::map<short, char> getLuaVal(int idx) { return getMapVal<short, char>(idx); }
	template<> inline std::map<short, char*> getLuaVal(int idx) { return getMapVal<short, char*>(idx); }
	template<> inline std::map<short, const char*> getLuaVal(int idx) { return getMapVal<short, const char*>(idx); }
	template<> inline std::map<short, float> getLuaVal(int idx) { return getMapVal<short, float>(idx); }
	template<> inline std::map<short, double> getLuaVal(int idx) { return getMapVal<short, double>(idx); }
	template<> inline std::map<short, short> getLuaVal(int idx) { return getMapVal<short, short>(idx); }
	template<> inline std::map<short, int> getLuaVal(int idx) { return getMapVal<short, int>(idx); }
	template<> inline std::map<short, long> getLuaVal(int idx) { return getMapVal<short, long>(idx); }
	template<> inline std::map<short, long long> getLuaVal(int idx) { return getMapVal<short, long long>(idx); }
	template<> inline std::map<short, unsigned char> getLuaVal(int idx) { return getMapVal<short, unsigned char>(idx); }
	template<> inline std::map<short, unsigned short> getLuaVal(int idx) { return getMapVal<short, unsigned short>(idx); }
	template<> inline std::map<short, unsigned int> getLuaVal(int idx) { return getMapVal<short, unsigned int>(idx); }
	template<> inline std::map<short, unsigned long> getLuaVal(int idx) { return getMapVal<short, unsigned long>(idx); }
	template<> inline std::map<short, unsigned long long> getLuaVal(int idx) { return getMapVal<short, unsigned long long>(idx); }
	template<> inline std::map<unsigned short, char> getLuaVal(int idx) { return getMapVal<unsigned short, char>(idx); }
	template<> inline std::map<unsigned short, char*> getLuaVal(int idx) { return getMapVal<unsigned short, char*>(idx); }
	template<> inline std::map<unsigned short, const char*> getLuaVal(int idx) { return getMapVal<unsigned short, const char*>(idx); }
	template<> inline std::map<unsigned short, float> getLuaVal(int idx) { return getMapVal<unsigned short, float>(idx); }
	template<> inline std::map<unsigned short, double> getLuaVal(int idx) { return getMapVal<unsigned short, double>(idx); }
	template<> inline std::map<unsigned short, short> getLuaVal(int idx) { return getMapVal<unsigned short, short>(idx); }
	template<> inline std::map<unsigned short, int> getLuaVal(int idx) { return getMapVal<unsigned short, int>(idx); }
	template<> inline std::map<unsigned short, long> getLuaVal(int idx) { return getMapVal<unsigned short, long>(idx); }
	template<> inline std::map<unsigned short, long long> getLuaVal(int idx) { return getMapVal<unsigned short, long long>(idx); }
	template<> inline std::map<unsigned short, unsigned char> getLuaVal(int idx) { return getMapVal<unsigned short, unsigned char>(idx); }
	template<> inline std::map<unsigned short, unsigned short> getLuaVal(int idx) { return getMapVal<unsigned short, unsigned short>(idx); }
	template<> inline std::map<unsigned short, unsigned int> getLuaVal(int idx) { return getMapVal<unsigned short, unsigned int>(idx); }
	template<> inline std::map<unsigned short, unsigned long> getLuaVal(int idx) { return getMapVal<unsigned short, unsigned long>(idx); }
	template<> inline std::map<unsigned short, unsigned long long> getLuaVal(int idx) { return getMapVal<unsigned short, unsigned long long>(idx); }
	template<> inline std::map<int, char> getLuaVal(int idx) { return getMapVal<int, char>(idx); }
	template<> inline std::map<int, char*> getLuaVal(int idx) { return getMapVal<int, char*>(idx); }
	template<> inline std::map<int, const char*> getLuaVal(int idx) { return getMapVal<int, const char*>(idx); }
	template<> inline std::map<int, float> getLuaVal(int idx) { return getMapVal<int, float>(idx); }
	template<> inline std::map<int, double> getLuaVal(int idx) { return getMapVal<int, double>(idx); }
	template<> inline std::map<int, short> getLuaVal(int idx) { return getMapVal<int, short>(idx); }
	template<> inline std::map<int, int> getLuaVal(int idx) { return getMapVal<int, int>(idx); }
	template<> inline std::map<int, long> getLuaVal(int idx) { return getMapVal<int, long>(idx); }
	template<> inline std::map<int, long long> getLuaVal(int idx) { return getMapVal<int, long long>(idx); }
	template<> inline std::map<int, unsigned char> getLuaVal(int idx) { return getMapVal<int, unsigned char>(idx); }
	template<> inline std::map<int, unsigned short> getLuaVal(int idx) { return getMapVal<int, unsigned short>(idx); }
	template<> inline std::map<int, unsigned int> getLuaVal(int idx) { return getMapVal<int, unsigned int>(idx); }
	template<> inline std::map<int, unsigned long> getLuaVal(int idx) { return getMapVal<int, unsigned long>(idx); }
	template<> inline std::map<int, unsigned long long> getLuaVal(int idx) { return getMapVal<int, unsigned long long>(idx); }
	template<> inline std::map<unsigned int, char> getLuaVal(int idx) { return getMapVal<unsigned int, char>(idx); }
	template<> inline std::map<unsigned int, char*> getLuaVal(int idx) { return getMapVal<unsigned int, char*>(idx); }
	template<> inline std::map<unsigned int, const char*> getLuaVal(int idx) { return getMapVal<unsigned int, const char*>(idx); }
	template<> inline std::map<unsigned int, float> getLuaVal(int idx) { return getMapVal<unsigned int, float>(idx); }
	template<> inline std::map<unsigned int, double> getLuaVal(int idx) { return getMapVal<unsigned int, double>(idx); }
	template<> inline std::map<unsigned int, short> getLuaVal(int idx) { return getMapVal<unsigned int, short>(idx); }
	template<> inline std::map<unsigned int, int> getLuaVal(int idx) { return getMapVal<unsigned int, int>(idx); }
	template<> inline std::map<unsigned int, long> getLuaVal(int idx) { return getMapVal<unsigned int, long>(idx); }
	template<> inline std::map<unsigned int, long long> getLuaVal(int idx) { return getMapVal<unsigned int, long long>(idx); }
	template<> inline std::map<unsigned int, unsigned char> getLuaVal(int idx) { return getMapVal<unsigned int, unsigned char>(idx); }
	template<> inline std::map<unsigned int, unsigned short> getLuaVal(int idx) { return getMapVal<unsigned int, unsigned short>(idx); }
	template<> inline std::map<unsigned int, unsigned int> getLuaVal(int idx) { return getMapVal<unsigned int, unsigned int>(idx); }
	template<> inline std::map<unsigned int, unsigned long> getLuaVal(int idx) { return getMapVal<unsigned int, unsigned long>(idx); }
	template<> inline std::map<unsigned int, unsigned long long> getLuaVal(int idx) { return getMapVal<unsigned int, unsigned long long>(idx); }
	template<> inline std::map<long, char> getLuaVal(int idx) { return getMapVal<long, char>(idx); }
	template<> inline std::map<long, char*> getLuaVal(int idx) { return getMapVal<long, char*>(idx); }
	template<> inline std::map<long, const char*> getLuaVal(int idx) { return getMapVal<long, const char*>(idx); }
	template<> inline std::map<long, float> getLuaVal(int idx) { return getMapVal<long, float>(idx); }
	template<> inline std::map<long, double> getLuaVal(int idx) { return getMapVal<long, double>(idx); }
	template<> inline std::map<long, short> getLuaVal(int idx) { return getMapVal<long, short>(idx); }
	template<> inline std::map<long, int> getLuaVal(int idx) { return getMapVal<long, int>(idx); }
	template<> inline std::map<long, long> getLuaVal(int idx) { return getMapVal<long, long>(idx); }
	template<> inline std::map<long, long long> getLuaVal(int idx) { return getMapVal<long, long long>(idx); }
	template<> inline std::map<long, unsigned char> getLuaVal(int idx) { return getMapVal<long, unsigned char>(idx); }
	template<> inline std::map<long, unsigned short> getLuaVal(int idx) { return getMapVal<long, unsigned short>(idx); }
	template<> inline std::map<long, unsigned int> getLuaVal(int idx) { return getMapVal<long, unsigned int>(idx); }
	template<> inline std::map<long, unsigned long> getLuaVal(int idx) { return getMapVal<long, unsigned long>(idx); }
	template<> inline std::map<long, unsigned long long> getLuaVal(int idx) { return getMapVal<long, unsigned long long>(idx); }
	template<> inline std::map<unsigned long, char> getLuaVal(int idx) { return getMapVal<unsigned long, char>(idx); }
	template<> inline std::map<unsigned long, char*> getLuaVal(int idx) { return getMapVal<unsigned long, char*>(idx); }
	template<> inline std::map<unsigned long, const char*> getLuaVal(int idx) { return getMapVal<unsigned long, const char*>(idx); }
	template<> inline std::map<unsigned long, float> getLuaVal(int idx) { return getMapVal<unsigned long, float>(idx); }
	template<> inline std::map<unsigned long, double> getLuaVal(int idx) { return getMapVal<unsigned long, double>(idx); }
	template<> inline std::map<unsigned long, short> getLuaVal(int idx) { return getMapVal<unsigned long, short>(idx); }
	template<> inline std::map<unsigned long, int> getLuaVal(int idx) { return getMapVal<unsigned long, int>(idx); }
	template<> inline std::map<unsigned long, long> getLuaVal(int idx) { return getMapVal<unsigned long, long>(idx); }
	template<> inline std::map<unsigned long, long long> getLuaVal(int idx) { return getMapVal<unsigned long, long long>(idx); }
	template<> inline std::map<unsigned long, unsigned char> getLuaVal(int idx) { return getMapVal<unsigned long, unsigned char>(idx); }
	template<> inline std::map<unsigned long, unsigned short> getLuaVal(int idx) { return getMapVal<unsigned long, unsigned short>(idx); }
	template<> inline std::map<unsigned long, unsigned int> getLuaVal(int idx) { return getMapVal<unsigned long, unsigned int>(idx); }
	template<> inline std::map<unsigned long, unsigned long> getLuaVal(int idx) { return getMapVal<unsigned long, unsigned long>(idx); }
	template<> inline std::map<unsigned long, unsigned long long> getLuaVal(int idx) { return getMapVal<unsigned long, unsigned long long>(idx); }
	template<> inline std::map<long long, char> getLuaVal(int idx) { return getMapVal<long long, char>(idx); }
	template<> inline std::map<long long, char*> getLuaVal(int idx) { return getMapVal<long long, char*>(idx); }
	template<> inline std::map<long long, const char*> getLuaVal(int idx) { return getMapVal<long long, const char*>(idx); }
	template<> inline std::map<long long, float> getLuaVal(int idx) { return getMapVal<long long, float>(idx); }
	template<> inline std::map<long long, double> getLuaVal(int idx) { return getMapVal<long long, double>(idx); }
	template<> inline std::map<long long, short> getLuaVal(int idx) { return getMapVal<long long, short>(idx); }
	template<> inline std::map<long long, int> getLuaVal(int idx) { return getMapVal<long long, int>(idx); }
	template<> inline std::map<long long, long> getLuaVal(int idx) { return getMapVal<long long, long>(idx); }
	template<> inline std::map<long long, long long> getLuaVal(int idx) { return getMapVal<long long, long long>(idx); }
	template<> inline std::map<long long, unsigned char> getLuaVal(int idx) { return getMapVal<long long, unsigned char>(idx); }
	template<> inline std::map<long long, unsigned short> getLuaVal(int idx) { return getMapVal<long long, unsigned short>(idx); }
	template<> inline std::map<long long, unsigned int> getLuaVal(int idx) { return getMapVal<long long, unsigned int>(idx); }
	template<> inline std::map<long long, unsigned long> getLuaVal(int idx) { return getMapVal<long long, unsigned long>(idx); }
	template<> inline std::map<long long, unsigned long long> getLuaVal(int idx) { return getMapVal<long long, unsigned long long>(idx); }
	template<> inline std::map<unsigned long long, char> getLuaVal(int idx) { return getMapVal<unsigned long long, char>(idx); }
	template<> inline std::map<unsigned long long, char*> getLuaVal(int idx) { return getMapVal<unsigned long long, char*>(idx); }
	template<> inline std::map<unsigned long long, const char*> getLuaVal(int idx) { return getMapVal<unsigned long long, const char*>(idx); }
	template<> inline std::map<unsigned long long, float> getLuaVal(int idx) { return getMapVal<unsigned long long, float>(idx); }
	template<> inline std::map<unsigned long long, double> getLuaVal(int idx) { return getMapVal<unsigned long long, double>(idx); }
	template<> inline std::map<unsigned long long, short> getLuaVal(int idx) { return getMapVal<unsigned long long, short>(idx); }
	template<> inline std::map<unsigned long long, int> getLuaVal(int idx) { return getMapVal<unsigned long long, int>(idx); }
	template<> inline std::map<unsigned long long, long> getLuaVal(int idx) { return getMapVal<unsigned long long, long>(idx); }
	template<> inline std::map<unsigned long long, long long> getLuaVal(int idx) { return getMapVal<unsigned long long, long long>(idx); }
	template<> inline std::map<unsigned long long, unsigned char> getLuaVal(int idx) { return getMapVal<unsigned long long, unsigned char>(idx); }
	template<> inline std::map<unsigned long long, unsigned short> getLuaVal(int idx) { return getMapVal<unsigned long long, unsigned short>(idx); }
	template<> inline std::map<unsigned long long, unsigned int> getLuaVal(int idx) { return getMapVal<unsigned long long, unsigned int>(idx); }
	template<> inline std::map<unsigned long long, unsigned long> getLuaVal(int idx) { return getMapVal<unsigned long long, unsigned long>(idx); }
	template<> inline std::map<unsigned long long, unsigned long long> getLuaVal(int idx) { return getMapVal<unsigned long long, unsigned long long>(idx); }

	template<typename T>
	struct SetUserData { inline static void setUserData(T t) {} };

	template<typename T>
	struct SetUserData<T*>
	{
		inline static void setUserData(T* t)
		{
			if (!t)
			{
				lua_pushnil(gLuaSgate);
				return;
			}
			ObjData<T>** od = (ObjData<T>**)lua_newuserdata(gLuaSgate, sizeof(ObjData<T>*));
			*od = new ObjData<T>(t);
			luaL_getmetatable(gLuaSgate, GlobalName<T>::getName());
			lua_setmetatable(gLuaSgate, -2);
		}
	};

	template<typename T> inline void setLuaVal(T t) { SetUserData<T>::setUserData(t); }
	template<> inline void setLuaVal(bool val) { lua_pushboolean(gLuaSgate, val); }
	template<> inline void setLuaVal(char val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(unsigned char val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(short val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(unsigned short val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(long val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(unsigned long val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(int val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(unsigned int val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(long long val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(unsigned long long val) { lua_pushinteger(gLuaSgate, val); }
	template<> inline void setLuaVal(float val) { lua_pushnumber(gLuaSgate, val); }
	template<> inline void setLuaVal(double val) { lua_pushnumber(gLuaSgate, val); }
	template<> inline void setLuaVal(char* val) { lua_pushstring(gLuaSgate, val); }
	template<> inline void setLuaVal(const char* val) { lua_pushstring(gLuaSgate, val); }
	template<> inline void setLuaVal(std::string val) { lua_pushlstring(gLuaSgate, val.c_str(), val.size()); }
	template<typename T>
	inline void setVectorVal(const std::vector<T>& vec)
	{
		lua_newtable(gLuaSgate);
		int i = 0;
		for (auto& e : vec)
		{
			setLuaVal(vec[i]);
			lua_rawseti(gLuaSgate, 2, i + 1);
			i++;
		}
		lua_pushvalue(gLuaSgate, -1);
	}
	template<typename T> inline void setLuaVal(std::vector<T> vec) { setVectorVal(vec); }

	template<typename K, typename V>
	inline void setMapVal(const std::map<K, V>& tab)
	{
		lua_newtable(gLuaSgate);
		for (auto& e : tab)
		{
			setLuaVal(e.first);
			setLuaVal(e.second);
			lua_rawset(gLuaSgate, 2);
		}
		lua_pushvalue(gLuaSgate, -1);
	}
	template<typename K, typename V> inline void setLuaVal(std::map<K, V> tab) { setMapVal(tab); }

	//	objfunc
	template<typename R, typename T>
	struct ObjectFunction_0 : public BaseFunc
	{
		typedef R(T::* Func)();
		Func func;
		ObjectFunction_0(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			T* obj = getLuaVal<T*>(1);
			if constexpr (std::is_same<void, R>::value)
			{
				(obj->*func)();
			}
			else
			{
				setLuaVal((obj->*func)());
			}
			return 1;
		}
	};

	template<typename R, typename T, typename Arg1>
	struct ObjectFunction_1 : public BaseFunc
	{
		typedef R(T::* Func)(Arg1);
		Func func;
		ObjectFunction_1(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			T* obj = getLuaVal<T*>(1);
			if constexpr (std::is_same<void, R>::value)
			{
				(obj->*func)(getLuaVal<Arg1>(2));
			}
			else
			{
				setLuaVal((obj->*func)(getLuaVal<Arg1>(2)));
			}
			return 1;
		}
	};

	template<typename R, typename T, typename Arg1, typename Arg2>
	struct ObjectFunction_2 : public BaseFunc
	{
		typedef R(T::* Func)(Arg1, Arg2);
		Func func;
		ObjectFunction_2(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			T* obj = getLuaVal<T*>(1);
			if constexpr (std::is_same<void, R>::value)
			{
				(obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3));
			}
			else
			{
				setLuaVal((obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3)));
			}
			return 0;
		}
	};

	template<typename R, typename T, typename Arg1, typename Arg2, typename Arg3>
	struct ObjectFunction_3 : public BaseFunc
	{
		typedef R(T::* Func)(Arg1, Arg2, Arg3);
		Func func;
		ObjectFunction_3(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			T* obj = getLuaVal<T*>(1);
			if constexpr (std::is_same<void, R>::value)
			{
				if constexpr (std::is_same<Arg3, LuaMsg&>::value)
				{
					LuaMsg lm;
					lm.name_ = getLuaVal<Arg2>(3);
					lm.data_ = Codec::encoder(lm.name_, gLuaSgate, 3, lm.lens_);
					(obj->*func)(getLuaVal<Arg1>(2), lm.name_, lm);
				}
				else
				{
					(obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3), getLuaVal<Arg3>(4));
				}
			}
			else
			{
				setLuaVal((obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3), getLuaVal<Arg3>(4)));
			}
			return 1;
		}
	};

	template<typename R, typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	struct ObjectFunction_4 : public BaseFunc
	{
		typedef R(T::* Func)(Arg1, Arg2, Arg3, Arg4);
		Func func;
		ObjectFunction_4(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			T* obj = getLuaVal<T*>(1);
			if constexpr (std::is_same<void, R>::value)
			{
				(obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3), getLuaVal<Arg3>(4), getLuaVal<Arg4>(5));
			}
			else
			{
				setLuaVal((obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3), getLuaVal<Arg3>(4), getLuaVal<Arg4>(5)));
			}
			return 1;
		}
	};

	template<typename R, typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	struct ObjectFunction_5 : public BaseFunc
	{
		typedef R(T::* Func)(Arg1, Arg2, Arg3, Arg4, Arg5);
		Func func;
		ObjectFunction_5(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			T* obj = getLuaVal<T*>(L, 1);
			if constexpr (std::is_same<void, R>::value)
			{
				(obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3), getLuaVal<Arg3>(4), getLuaVal<Arg4>(5), getLuaVal<Arg5>(6));
			}
			else
			{
				setLuaVal((obj->*func)(getLuaVal<Arg1>(2), getLuaVal<Arg2>(3), getLuaVal<Arg3>(4), getLuaVal<Arg4>(5), getLuaVal<Arg5>(6)));
			}
			return 1;
		}
	};

	// func
	template<typename R>
	struct Function_0 : public BaseFunc
	{
		typedef R(*Func)();
		Func func;
		Function_0(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			if constexpr (std::is_same<void, R>::value)
			{
				(*func)();
			}
			else
			{
				setLuaVal((*func)());
			}

			return 1;
		}
	};

	template<typename R, typename Arg1>
	struct Function_1 : public BaseFunc
	{
		typedef R(*Func)(Arg1);
		Func func;
		Function_1(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			if constexpr (std::is_same<void, R>::value)
			{
				(*func)(getLuaVal<Arg1>(1));
			}
			else
			{
				setLuaVal((*func)(getLuaVal<Arg1>(1)));
			}
			return 1;
		}
	};

	template<typename R, typename Arg1, typename Arg2>
	struct Function_2 : public BaseFunc
	{
		typedef R(*Func)(Arg1, Arg2);
		Func func;
		Function_2(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			if constexpr (std::is_same<void, R>::value)
			{
				(*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2));
			}
			else
			{
				setLuaVal((*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2)));
			}
			return 1;
		}
	};

	template<typename R, typename Arg1, typename Arg2, typename Arg3>
	struct Function_3 : public BaseFunc
	{
		typedef R(*Func)(Arg1, Arg2, Arg3);
		Func func;
		Function_3(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			if constexpr (std::is_same<void, R>::value)
			{
				(*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2), getLuaVal<Arg2>(3));
			}
			else
			{
				setLuaVal((*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2), getLuaVal<Arg3>(3)));
			}
			return 1;
		}
	};

	template<typename R, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	struct Function_4 : public BaseFunc
	{
		typedef R(*Func)(Arg1, Arg2, Arg3, Arg4);
		Func func;
		Function_4(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			if constexpr (std::is_same<void, R>::value)
			{
				(*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2), getLuaVal<Arg2>(3), getLuaVal<Arg2>(4));
			}
			else
			{
				setLuaVal((*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2), getLuaVal<Arg3>(3), getLuaVal<Arg4>(4)));
			}
			return 1;
		}
	};

	template<typename R, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	struct Function_5 : public BaseFunc
	{
		typedef R(*Func)(Arg1, Arg2, Arg3, Arg4, Arg5);
		Func func;
		Function_5(Func func) { this->func = func; }
		int call(lua_State* L)
		{
			if constexpr (std::is_same<void, R>::value)
			{
				(*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2), getLuaVal<Arg3>(3), getLuaVal<Arg4>(4), getLuaVal<Arg5>(5));
			}
			else
			{
				setLuaVal((*func)(getLuaVal<Arg1>(1), getLuaVal<Arg2>(2), getLuaVal<Arg3>(3), getLuaVal<Arg4>(4), getLuaVal<Arg5>(5)));
			}
			return 1;
		}
	};


	inline int luaCall(lua_State* l)
	{
		BaseFunc* func = (BaseFunc*)lua_touserdata(gLuaSgate, lua_upvalueindex(1));
		return func->call(gLuaSgate);
	}

	template<typename T>
	inline int luaGc(lua_State* l)
	{
		ObjData<T>** ud = (ObjData<T>**)luaL_checkudata(gLuaSgate, -1, GlobalName<T>::getName());
		std::cout << typeid(*ud).name() << std::endl;
		delete (*ud);
		*ud = nullptr;
		return 0;
	}

	template<typename T>
	inline void regClass(const char* name)
	{
		GlobalName<T>::setName(name);

		luaL_newmetatable(gLuaSgate, name);

		lua_pushstring(gLuaSgate, "__gc");
		lua_pushcfunction(gLuaSgate, &luaGc<T>);
		lua_rawset(gLuaSgate, -3);

		lua_pushstring(gLuaSgate, "__index");
		lua_pushvalue(gLuaSgate, -2);
		lua_rawset(gLuaSgate, -3);

		lua_pushstring(gLuaSgate, "__newindex");
		lua_pushvalue(gLuaSgate, -2);
		lua_rawset(gLuaSgate, -3);

		lua_settop(gLuaSgate, 0);
	}

	template<typename T>
	inline void regGlobalVar(const char* name, T t)
	{
		setLuaVal(t);
		lua_setglobal(gLuaSgate, name);
	}

	// regobjfunc
	template<typename T, typename R>
	inline void regClassFunc(const char* name, R(T::* func)())
	{
		luaL_getmetatable(gLuaSgate, GlobalName<T>::getName());
		if (lua_istable(gLuaSgate, -1))
		{
			ObjectFunction_0<R, T>* funcs = new ObjectFunction_0<R, T>(func);
			lua_pushstring(gLuaSgate, name);
			lua_pushlightuserdata(gLuaSgate, funcs);
			lua_pushcclosure(gLuaSgate, &luaCall, 1);
			lua_rawset(gLuaSgate, -3);
		}
		else
		{
			logDebug("no reg class = %s", GlobalName<T>::getName());
		}
		lua_settop(gLuaSgate, 0);
	}

	template<typename T, typename R, typename Arg1>
	inline void regClassFunc(const char* name, R(T::* func)(Arg1))
	{
		luaL_getmetatable(gLuaSgate, GlobalName<T>::getName());
		if (lua_istable(gLuaSgate, -1))
		{
			ObjectFunction_1<R, T, Arg1>* funcs = new ObjectFunction_1<R, T, Arg1>(func);
			lua_pushstring(gLuaSgate, name);
			lua_pushlightuserdata(gLuaSgate, funcs);
			lua_pushcclosure(gLuaSgate, &luaCall, 1);
			lua_rawset(gLuaSgate, -3);
		}
		else
		{
			logDebug("no reg class = %s", GlobalName<T>::getName());
		}
		lua_settop(gLuaSgate, 0);
	}

	template<typename T, typename R, typename Arg1, typename Arg2>
	inline void regClassFunc(const char* name, R(T::* func)(Arg1, Arg2))
	{
		luaL_getmetatable(gLuaSgate, GlobalName<T>::getName());
		if (lua_istable(gLuaSgate, -1))
		{
			ObjectFunction_2<R, T, Arg1, Arg2>* funcs = new ObjectFunction_2<R, T, Arg1, Arg2>(func);
			lua_pushstring(gLuaSgate, name);
			lua_pushlightuserdata(gLuaSgate, funcs);
			lua_pushcclosure(gLuaSgate, &luaCall, 1);
			lua_rawset(gLuaSgate, -3);
		}
		else
		{
			logDebug("no reg class = %s", GlobalName<T>::getName());
		}
		lua_settop(gLuaSgate, 0);
	}

	template<typename T, typename R, typename Arg1, typename Arg2, typename Arg3>
	inline void regClassFunc(const char* name, R(T::* func)(Arg1, Arg2, Arg3))
	{
		luaL_getmetatable(gLuaSgate, GlobalName<T>::getName());
		if (lua_istable(gLuaSgate, -1))
		{
			ObjectFunction_3<R, T, Arg1, Arg2, Arg3>* funcs = new ObjectFunction_3<R, T, Arg1, Arg2, Arg3>(func);
			lua_pushstring(gLuaSgate, name);
			lua_pushlightuserdata(gLuaSgate, funcs);
			lua_pushcclosure(gLuaSgate, &luaCall, 1);
			lua_rawset(gLuaSgate, -3);
		}
		else
		{
			logDebug("no reg class = %s", GlobalName<T>::getName());
		}

		lua_settop(gLuaSgate, 0);
	}

	template<typename T, typename R, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	inline void regClassFunc(const char* name, R(T::* func)(Arg1, Arg2, Arg3, Arg4))
	{
		luaL_getmetatable(gLuaSgate, GlobalName<T>::getName());
		if (lua_istable(gLuaSgate, -1))
		{
			ObjectFunction_4<R, T, Arg1, Arg2, Arg3, Arg4>* funcs = new ObjectFunction_4<R, T, Arg1, Arg2, Arg3, Arg4>(func);
			lua_pushstring(gLuaSgate, name);
			lua_pushlightuserdata(gLuaSgate, funcs);
			lua_pushcclosure(gLuaSgate, &luaCall, 1);
			lua_rawset(gLuaSgate, -3);
		}
		else
		{
			logDebug("no reg class = %s", GlobalName<T>::getName());
		}
		lua_settop(gLuaSgate, 0);
	}

	template<typename T, typename R, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	inline void regClassFunc(const char* name, R(T::* func)(Arg1, Arg2, Arg3, Arg4, Arg5))
	{
		luaL_getmetatable(gLuaSgate, GlobalName<T>::getName());
		if (lua_istable(gLuaSgate, -1))
		{
			ObjectFunction_5<R, T, Arg1, Arg2, Arg3, Arg4, Arg5>* funcs = new ObjectFunction_5<R, T, Arg1, Arg2, Arg3, Arg4, Arg5>(func);
			lua_pushstring(gLuaSgate, name);
			lua_pushlightuserdata(gLuaSgate, funcs);
			lua_pushcclosure(gLuaSgate, &luaCall, 1);
			lua_rawset(gLuaSgate, -3);
		}
		else
		{
			logDebug("no reg class = %s", GlobalName<T>::getName());
		}
		lua_settop(gLuaSgate, 0);
	}

	// regfunc
	template<typename R>
	inline void regGloablFunc(const char* name, R(*func)())
	{
		Function_0<R>* funcs = new Function_0<R>(func);
		lua_pushlightuserdata(gLuaSgate, funcs);
		lua_pushcclosure(gLuaSgate, luaCall, 1);
		lua_setglobal(gLuaSgate, name);
	}

	template<typename R, typename Arg1>
	inline void regGloablFunc(const char* name, R(*func)(Arg1))
	{
		Function_1<R, Arg1>* funcs = new Function_1<R, Arg1>(func);
		lua_pushlightuserdata(gLuaSgate, funcs);
		lua_pushcclosure(gLuaSgate, luaCall, 1);
		lua_setglobal(gLuaSgate, name);
	}

	template<typename R, typename Arg1, typename Arg2>
	inline void regGloablFunc(const char* name, R(*func)(Arg1, Arg2))
	{
		Function_2<R, Arg1, Arg2>* funcs = new Function_2<R, Arg1, Arg2>(func);
		lua_pushlightuserdata(gLuaSgate, funcs);
		lua_pushcclosure(gLuaSgate, luaCall, 1);
		lua_setglobal(gLuaSgate, name);
	}

	template<typename R, typename Arg1, typename Arg2, typename Arg3>
	inline void regGloablFunc(const char* name, R(*func)(Arg1, Arg2, Arg3))
	{
		Function_3<R, Arg1, Arg2, Arg3>* funcs = new Function_3<R, Arg1, Arg2, Arg3>(func);
		lua_pushlightuserdata(gLuaSgate, funcs);
		lua_pushcclosure(gLuaSgate, luaCall, 1);
		lua_setglobal(gLuaSgate, name);
	}

	template<typename R, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
	inline void regGloablFunc(const char* name, R(*func)(Arg1, Arg2, Arg3, Arg4))
	{
		Function_4<R, Arg1, Arg2, Arg3, Arg4>* funcs = new Function_4<R, Arg1, Arg2, Arg3, Arg4>(func);
		lua_pushlightuserdata(gLuaSgate, funcs);
		lua_pushcclosure(gLuaSgate, luaCall, 1);
		lua_setglobal(gLuaSgate, name);
	}

	template<typename R, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
	inline void regGloablFunc(const char* name, R(*func)(Arg1, Arg2, Arg3, Arg4, Arg5))
	{
		Function_5<R, Arg1, Arg2, Arg3, Arg4, Arg5>* funcs = new Function_5<R, Arg1, Arg2, Arg3, Arg4, Arg5>(func);
		lua_pushlightuserdata(gLuaSgate, funcs);
		lua_pushcclosure(gLuaSgate, luaCall, 1);
		lua_setglobal(gLuaSgate, name);
	}

	struct CallLua
	{
		int index = 0;
		bool exist = false;

		bool existFunc() { return exist; }
		~CallLua() { if (exist) luaL_unref(gLuaSgate, LUA_REGISTRYINDEX, index); }
		CallLua(const char* func, const char* moduleName = nullptr)
		{
			if (!func) { logErr("CallLua no func"); return; }

			if (moduleName)
			{
				lua_getglobal(gLuaSgate, moduleName);
				if (!lua_istable(gLuaSgate, -1))
				{
					logDebug("no find this module = %s", moduleName);
					return;
				}
				lua_getfield(gLuaSgate, -1, func);
				if (!lua_isfunction(gLuaSgate, -1))
				{
					logDebug("no find shis func = %s!", func);
					return;
				}
			}
			else
			{
				lua_getglobal(gLuaSgate, func);
				if (!lua_isfunction(gLuaSgate, -1))
				{
					logDebug("no find global func", func);
					return;
				}
			}

			index = luaL_ref(gLuaSgate, LUA_REGISTRYINDEX);
			exist = true;
		}

		template < size_t I = 0, typename FuncT, typename ...Tp>
		inline typename std::enable_if_t<I == sizeof ...(Tp)> for_each(std::tuple<Tp ...>&, FuncT)
		{
		}

		template < size_t I = 0, typename FuncT, typename ...Tp>
		inline typename std::enable_if_t < I < sizeof ...(Tp)> for_each(std::tuple<Tp ...>& t, FuncT f)
		{
			f(std::get<I>(t));
			for_each<I + 1, FuncT, Tp...>(t, f);
		}

		template<typename R, typename S, typename ...Args>
		R call(Args... args)
		{
			lua_pushcclosure(gLuaSgate, errFunc, 0);
			lua_rawgeti(gLuaSgate, LUA_REGISTRYINDEX, index);
			auto valList = std::forward_as_tuple(args...);

			if constexpr (std::is_same<S, LuaMsg>::value)
			{
				setLuaVal(std::get<0>(valList));
				Codec::decode(std::get<1>(valList), std::get<2>(valList), std::get<3>(valList), gLuaSgate);
				lua_pcall(gLuaSgate, 2, 1, 1);
				lua_settop(gLuaSgate, 0);
			}
			else
			{
				for_each(valList, [&](auto& e)
					{
						setLuaVal(e);
					});
				int nargs = (int)sizeof...(args);
				lua_pcall(gLuaSgate, nargs, 1, 1);
				if constexpr (!std::is_same<void, R>::value)
				{
					R result = getLuaVal<R>(lua_gettop(gLuaSgate));
					lua_settop(gLuaSgate, 0);
					return result;
				}
				else
				{
					lua_settop(gLuaSgate, 0);
				}
			}

		}
	};
}


#define Reg_Class(cls) LuaBind::regClass<cls>(#cls)
#define Reg_Class_Func(cls, func) LuaBind::regClassFunc<cls>(#func, &cls::func)
#define Reg_Gloabl_Func(func) LuaBind::regGloablFunc(#func, &func) 
#define Reg_Global_Var_Same(var) LuaBind::regGlobalVar(#var, var) 
#define Reg_Global_Var_No_Same(var, name) LuaBind::regGlobalVar(name, var)   

