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
	dbs::Dbers::addMysql(0, "192.168.1.105", 3306, "david", "123456", "poker", 3);
	// dbs::Dbers::addPostgresql(1, "192.168.1.105", 5432, "david", "123456", "pds", 3);

	// dbs::Dbers::addMssql(5, "localhost", 1433, "sa", "1234Qwer", "master", 3);

	dbs::Dater<User> dater;
	dbs::Dbers::gets(&dater, 0, "select `rid`, `name`, `coin`, `gold`, `rtime`, `ltime` from s_user");

	User *user = nullptr;
	while((user = dater.get()) != nullptr)
	{
		printf("------>rid:%d, name:%s, coin:%d, gold:%d, rtime:%d, ltime:%d\n", user->rid(), user->name().c_str(), user->coin(), user->gold(), user->rtime(), user->ltime());
	}

	// uint64_t n = dbs::Dbers::add(0, " insert into s_user(`rid`, `name`, `coin`, `gold`, `rtime`, `ltime`) values(1119999, 'cs_1119999', 5132546, 4235, 1591885126, 1591885126);");
	// printf("------>n:%d\n", n);


	std::mutex mtx;
	std::condition_variable cond;
	std::unique_lock<std::mutex> lck(mtx);
	cond.wait(lck);

	return 0;
}

