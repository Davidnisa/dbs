#include <stdio.h>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <chrono>

#include "dbs/dbers.h"

#include "dat/td.pb.h"

int main(int argc,char **argv)
{

	dbs::Dbers::init();
	dbs::Dbers::addMysql(0, "192.168.1.105", 3306, "david", "123456", "poker", 1);
	//dbs::Dbers::addPostgresql(1, "192.168.1.105", 5432, "david", "123456", "pds", 3);

	User *user = nullptr;

////////////////////////////////////////////////////////////////////

	dbs::Dater<User> dater_0;
	dbs::Dbers::gets(&dater_0, 0, "select `rid`, `name`, `coin`, `gold`, `rtime`, `ltime` from s_user where rid={0}", 1119999);
	if(dater_0.isError())
	{
		printf("------>dbs::Dbers::gets, error, sql:%s\n", dater_0.str().c_str());
	}
	else
	{
		while((user = dater_0.get()) != nullptr)
		{
			printf("------>dbs::Dbers::gets, rid:%d, name:%s, coin:%d, gold:%d, rtime:%d, ltime:%d\n", user->rid(), user->name().c_str(), user->coin(), user->gold(), user->rtime(), user->ltime());
		}
		dater_0.close();
	}

////////////////////////////////////////////////////////////////////

	User cond_1;
	cond_1.set_rid(1119999);

	dbs::Dater<User> dater_1;
	dbs::Dbers::getsd(&dater_1, 0, "select `rid`, `name`, `coin`, `gold`, `rtime`, `ltime` from s_user where rid={rid}", &cond_1);
	if(dater_1.isError())
	{
		printf("------>dbs::Dbers::getsd, error, sql:%s\n", dater_1.str().c_str());
	}
	else
	{
		while((user = dater_1.get()) != nullptr)
		{
			printf("------>dbs::Dbers::getsd, rid:%d, name:%s, coin:%d, gold:%d, rtime:%d, ltime:%d\n", user->rid(), user->name().c_str(), user->coin(), user->gold(), user->rtime(), user->ltime());
		}
		dater_1.close();
	}

////////////////////////////////////////////////////////////////////

	User cond_2;
	cond_2.set_rid(1119999);

	dbs::Dater<User> dater_2;
	dbs::Dbers::getsg(&dater_2, 0, "rid={0}", 1119999);
	if(dater_2.isError())
	{
		printf("------>dbs::Dbers::getsg, error, sql:%s\n", dater_2.str().c_str());
	}
	else
	{
		while((user = dater_2.get()) != nullptr)
		{
			printf("------>dbs::Dbers::getsg, rid:%d, name:%s, coin:%d, gold:%d, rtime:%d, ltime:%d\n", user->rid(), user->name().c_str(), user->coin(), user->gold(), user->rtime(), user->ltime());
		}
		dater_2.close();
	}

////////////////////////////////////////////////////////////////////

	dbs::Dater<User> dater_3;
	dbs::Dbers::getsgd(&dater_3, 0);
	if(dater_3.isError())
	{
		printf("------>dbs::Dbers::getsgd, error, sql:%s\n", dater_3.str().c_str());
	}
	else
	{
		while((user = dater_3.get()) != nullptr)
		{
			printf("------>dbs::Dbers::getsgd, rid:%d, name:%s, coin:%d, gold:%d, rtime:%d, ltime:%d\n", user->rid(), user->name().c_str(), user->coin(), user->gold(), user->rtime(), user->ltime());
		}
		dater_3.close();
	}

////////////////////////////////////////////////////////////////////

	User cond_4;
	cond_4.set_rid(1119999);

	dbs::Dater<User> dater_4;
	dbs::Dbers::getsgdc(&dater_4, 0, &cond_4);
	if(dater_4.isError())
	{
		printf("------>dbs::Dbers::getsgdc, error, sql:%s\n", dater_4.str().c_str());
	}
	else
	{
		while((user = dater_4.get()) != nullptr)
		{
			printf("------>dbs::Dbers::getsgdc, rid:%d, name:%s, coin:%d, gold:%d, rtime:%d, ltime:%d\n", user->rid(), user->name().c_str(), user->coin(), user->gold(), user->rtime(), user->ltime());
		}
		dater_4.close();
	}

////////////////////////////////////////////////////////////////////

	dbs::Result *rs = dbs::Dbers::add(0, " insert into s_user(`rid`, `name`, `coin`, `gold`, `rtime`, `ltime`) values(1111999, 'cs_1119999', 5132546, 4235, 1591885126, 1591885126);");
	if(rs->isError())
		printf("------>dbs::Dbers::add, error, sql:%s\n", rs->str().c_str());
	else
		printf("------>dbs::Dbers::add, ok, affected:%d\n", rs->affected());
	delete rs;

////////////////////////////////////////////////////////////////////

	User userr;
	userr.set_rid(1545652);

	rs = dbs::Dbers::addd(0, " insert into s_user(`rid`, `name`, `coin`, `gold`, `rtime`, `ltime`) values({rid}, {name}, {coin}, {gold}, {rtime}, {ltime});", &userr);
	if(rs->isError())
		printf("------>dbs::Dbers::addd, error, sql:%s\n", rs->str().c_str());
	else
		printf("------>dbs::Dbers::addd, ok, affected:%d\n", rs->affected());
	delete rs;

////////////////////////////////////////////////////////////////////

	rs = dbs::Dbers::addgd(0, &userr);
	if(rs->isError())
		printf("------>dbs::Dbers::addgd, error, sql:%s\n", rs->str().c_str());
	else
		printf("------>dbs::Dbers::addgd, ok, affected:%d\n", rs->affected());
	delete rs;

////////////////////////////////////////////////////////////////////

	rs = dbs::Dbers::update(0, "update s_user set name = {0} where rid = {0}", "xxx", 1111999);
	if(rs->isError())
		printf("------>dbs::Dbers::update, error, sql:%s\n", rs->str().c_str());
	else
		printf("------>dbs::Dbers::update, ok, affected:%d\n", rs->affected());
	delete rs;

////////////////////////////////////////////////////////////////////

	userr.set_name("xxxxxxx");

	rs = dbs::Dbers::updatedg(0, &userr, "rid={0}", 1545652);
	if(rs->isError())
		printf("------>dbs::Dbers::updatedg, error, sql:%s\n", rs->str().c_str());
	else
		printf("------>dbs::Dbers::updatedg, ok, affected:%d\n", rs->affected());
	delete rs;

////////////////////////////////////////////////////////////////////

	Cond cond1;
	cond1.set_rid(1545652);

	rs = dbs::Dbers::updatedgc(0, &userr, &cond1);
	if(rs->isError())
		printf("------>dbs::Dbers::updatedgc, error, sql:%s\n", rs->str().c_str());
	else
		printf("------>dbs::Dbers::updatedgc, ok, affected:%d\n", rs->affected());
	delete rs;

	std::mutex mtx;
	std::condition_variable cond;
	std::unique_lock<std::mutex> lck(mtx);
	cond.wait(lck);

	return 0;
}

