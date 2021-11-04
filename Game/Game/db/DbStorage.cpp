#include "DbStorage.h"

#include <iostream>
#include <typeinfo>

#include "../log/Log.h"
#include "../MainThread.h"
#include "../player/Player.h"

DbStorage::DbStorage()
{

}

DbStorage::~DbStorage()
{
	if (mysql_)
	{
		mysql_close(mysql_);
	}
}

void DbStorage::initDb(const char* host, const char* user, const char* passwd, const char* dbName, int port)
{
	//host_ = host;
	//user_ = user;
	//passwd_ = passwd;
	//dbName_ = dbName;
	//port_ = port;
	//mysql_ = mysql_init(nullptr);
	//if (!mysql_)
	//{
	//	logDebug("MySql::initDb err = %s", mysql_error(mysql_));
	//	exit(0);
	//}
	//if (!mysql_real_connect(mysql_, host_, user_, passwd_, dbName_, port_, nullptr, 0))
	//{
	//	logDebug("MySql::initDb err = %s", mysql_error(mysql_));
	//	exit(0);
	//}
}



void DbStorage::load()
{
	//char buf[100] = { 0 };
	//sprintf(buf, "select * from player where serverId = %d", 1);
	//if (mysql_query(mysql_, buf))
	//{
	//	logDebug("DbStorage::load err = %s", mysql_error(mysql_));
	//	return;
	//}


	//MYSQL_RES* res = mysql_store_result(mysql_);
	//if (res)
	//{
	//	MYSQL_ROW row = nullptr;
	//	int col = mysql_num_fields(res);
	//	while (row = mysql_fetch_row(res))
	//	{
	//		for (int i = 0; i < col; ++i)
	//		{
	//			cout << typeid(row[i]).name() << " " << row[i] << endl;
	//		}
	//	}
	//}

	//mysql_free_result(res);
}

void DbStorage::save()
{
	std::unique_lock<std::mutex> lk(gMainThread.mutex_);
	while (gMainThread.queueTask_.empty())
	{
		gMainThread.cond_.wait(lk);
	}

	DbTask t = gMainThread.queueTask_.front();
	gMainThread.queueTask_.pop();
	lk.unlock();

	if (t.data_)
	{
		//char buf[10000] = { 0 };
		//sprintf(buf, "update %s set %s = %s where serverId = %d", t.tableName_, t.fieldName_, t.data_, 1);
		//if (mysql_query(mysql_, buf))
		//{
		//	logDebug("DbStorage::load err = %s", mysql_error(mysql_));
		//	return;
		//}
	}
	else
	{
		if (t.player_)
		{
			//Player* p = t.player_;
			//std::string s = "update player set ";
			//s.append("id = ").append(std::to_string(p->getUid())).append(", ");
			//s.append("sessionId = ").append(std::to_string(p->getSessionId())).append(", ");
			//s.append("name = '").append("xxx").append("'");

			//s.append(" where serverId = ").append(std::to_string(1)).append(" and id = ").append(std::to_string(1));

			//if (mysql_query(mysql_, s.c_str()))
			//{
			//	logDebug("DbStorage::load err = %s", mysql_error(mysql_));
			//	return;
			//}
		}
	}
}
