#pragma once

#include <stdint.h>
#include "../Libs/mysql/mysql.h"
#include "../public/Singleton.h"


class DbStorage : public Singleton<DbStorage>
{
public:
	DbStorage();
	~DbStorage();

	void initDb(const char* host, const char* user, const char* passwd, const char* dbName, int port = 3306);
	void load();
	void save();
private:
	const char* host_;
	const char* user_;
	const char* passwd_;
	const char* dbName_;
	uint32_t port_;
	MYSQL* mysql_;
};

#define gDbStorage DbStorage::instance()