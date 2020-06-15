#include "dbers.h"

namespace dbs
{

/**mutex**/
std::mutex Dbers::_mtx;
/**wait mutex**/
std::mutex Dbers::_wmtx;
/**wait**/
std::condition_variable Dbers::_wcv;
/**dber socket**/
struct _dber_sock_t_ *Dbers::_head = nullptr;
/**dber socket**/
struct _dber_sock_t_ *Dbers::_rear = nullptr;
/**dbers**/
std::unordered_map<int32_t, struct _dber_t_ *> *Dbers::_dbers = nullptr;

/**format**/
std::string Cmd::formatGetsd(struct _dber_sock_t_ *sock, const ::google::protobuf::Descriptor *descriptor)
{
	std::string s;

	if(sock->_dber->_type == _dbs_mysql_ || sock->_dber->_type == _dbs_postgresql_ || sock->_dber->_type == _dbs_sqlite_)
	{
		s = "select ";

		int32_t n = descriptor->field_count();
		const ::google::protobuf::FieldDescriptor *field = nullptr;
		for(int32_t i = 0;i < n;i++)
		{
			field = descriptor->field(i);
			if(i > 0)
				s.append(", ");
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("`");
			s.append(field->name());
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("`");
		}

		std::string name = descriptor->name();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);
		s.append(" from ").append(name);
	}
	
	return s;
}

/**format**/
std::string Cmd::formatGetsdc(struct _dber_sock_t_ *sock, const ::google::protobuf::Descriptor *descriptor, ::google::protobuf::Message *cond)
{
	std::string s = Cmd::formatGetsd(sock, descriptor);
	std::string where = Cmd::formatCond(sock, cond);
	if(where.length() > 0)
		s.append(" ").append(where);
	s.append(";");
	return s;
}

/**format**/
std::string Cmd::formatAddgd(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat)
{
	std::string s;

	const ::google::protobuf::Descriptor *descriptor = dat->GetDescriptor();

	if(sock->_dber->_type == _dbs_mysql_ || sock->_dber->_type == _dbs_postgresql_ || sock->_dber->_type == _dbs_sqlite_)
	{
		s = "insert into ";
		std::string values = "values(";

		std::string name = descriptor->name();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		s.append(name);
		s.append("(");

		int32_t n = descriptor->field_count();
		const ::google::protobuf::FieldDescriptor *field = nullptr;
		for(int32_t i = 0;i < n;i++)
		{
			field = descriptor->field(i);
			if(i > 0)
			{
				s.append(", ");
				values.append(", ");
			}
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("`");
			s.append(field->name());
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("`");

			_cmt_format_append_dat_t_::append(values, sock, dat, field);
		}

		s.append(")");
		values.append(")");

		s.append(" ").append(values).append(";");
	}

	return s;
}

/**format**/
std::string Cmd::formatUpdatedgc(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat, ::google::protobuf::Message *cond)
{
	std::string s = Cmd::formatUpdateHead(sock, dat);
	std::string where = Cmd::formatCond(sock, cond);
	if(where.length() > 0)
		s.append(" ").append(where);
	s.append(";");
	return s;
}

/**format**/
std::string Cmd::formatRemovegd(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat)
{
	std::string s;

	const ::google::protobuf::Descriptor *descriptor = dat->GetDescriptor();

	if(sock->_dber->_type == _dbs_mysql_ || sock->_dber->_type == _dbs_postgresql_ || sock->_dber->_type == _dbs_sqlite_)
	{
		s = "delete from ";

		std::string name = descriptor->name();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		s.append(name);
	}

	std::string where = Cmd::formatCond(sock, dat);
	if(where.length() > 0)
		s.append(" ").append(where);
	s.append(";");
	
	return s;
}

/**format**/
std::string Cmd::formatUpdateHead(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat)
{
	std::string s;

	const ::google::protobuf::Descriptor *descriptor = dat->GetDescriptor();

	if(sock->_dber->_type == _dbs_mysql_ || sock->_dber->_type == _dbs_postgresql_ || sock->_dber->_type == _dbs_sqlite_)
	{
		s = "update ";

		std::string name = descriptor->name();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		s.append(name).append(" set ");

		int32_t n = descriptor->field_count();
		const ::google::protobuf::FieldDescriptor *field = nullptr;
		for(int32_t i = 0;i < n;i++)
		{
			field = descriptor->field(i);
			if(i > 0)
				s.append(",");
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("`");
			s.append(field->name());
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("` = ");

			_cmt_format_append_dat_t_::append(s, sock, dat, field);
		}
	}
	
	return s;
}

/**format**/
std::string Cmd::formatCond(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat)
{
	std::string s;

	const ::google::protobuf::Descriptor *descriptor = dat->GetDescriptor();

	if(sock->_dber->_type == _dbs_mysql_ || sock->_dber->_type == _dbs_postgresql_ || sock->_dber->_type == _dbs_sqlite_)
	{
		s = "where ";

		int32_t n = descriptor->field_count();
		const ::google::protobuf::FieldDescriptor *field = nullptr;
		for(int32_t i = 0;i < n;i++)
		{
			field = descriptor->field(i);
			if(i > 0)
				s.append(", ");
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("`");
			s.append(field->name());
			if(sock->_dber->_type == _dbs_mysql_)
				s.append("`");
			s.append(" = ");

			_cmt_format_append_dat_t_::append(s, sock, dat, field);
		}
	}
	
	return s;
}

Daterd::Daterd()
{

}

Daterd::~Daterd()
{

}

/**close**/
void Daterd::close()
{
	this->_descriptor = nullptr;
	this->_prototype = nullptr;

	if(this->_mysqlRss != nullptr)
		mysql_free_result(this->_mysqlRss);
	this->_mysqlRss = nullptr;
	
	this->_postgresqlRssI = 0;
	this->_postgresqlRssN = 0;
	if(this->_postgresqlRss != nullptr)
		PQclear(this->_postgresqlRss);
	this->_postgresqlRss = nullptr;

	if(this->_fieldDescriptors != nullptr)
	{
		for(int32_t i = 0;i < this->_fieldCount;i++)
		{
			if(this->_fieldDescriptors[i] != nullptr)
				this->_fieldDescriptors[i] = nullptr;
		}
		delete []this->_fieldDescriptors;
	}
	
	this->_fieldCount = 0;
	this->_fieldDescriptors = nullptr;

	if(this->_sock != nullptr)
		Dbers::release(this->_sock);
	this->_sock = nullptr;
}

/**init field from mysql**/
void Daterd::initFieldFromMysql()
{
	int32_t count = mysql_num_fields(this->_mysqlRss);
	MYSQL_FIELD *fields = mysql_fetch_fields(this->_mysqlRss);
	if(this->_fieldCount > 0 && count > this->_fieldCount)
	{
		int32_t i = 0;
		const ::google::protobuf::FieldDescriptor **descriptors = new const ::google::protobuf::FieldDescriptor *[count];

		for(i = 0;i < this->_fieldCount;i++)
		{
			descriptors[i] = this->_descriptor->FindFieldByName(fields[i].name);
			this->_fieldDescriptors[i] = nullptr;
		}
		delete []this->_fieldDescriptors;

		for(;i < count;i++)
			descriptors[i] = this->_descriptor->FindFieldByName(fields[i].name);

		this->_fieldCount = count;
		this->_fieldDescriptors = descriptors;
	}
	else
	{
		if(this->_fieldCount <= 0)
		{
			this->_fieldCount = count;
			this->_fieldDescriptors = new const ::google::protobuf::FieldDescriptor *[count];
		}
		int32_t i = 0;
		for(;i < count;i++)
			this->_fieldDescriptors[i] = this->_descriptor->FindFieldByName(fields[i].name);
		for(;i < this->_fieldCount;i++)
			this->_fieldDescriptors[i] = nullptr;
	}
}

/**init field from postgresql**/
void Daterd::initFieldFromPostgresql()
{
	this->_postgresqlRssI = 0;
	this->_postgresqlRssN = PQntuples(this->_postgresqlRss);
	int32_t count = PQnfields(this->_postgresqlRss);
	if(this->_fieldCount > 0 && count > this->_fieldCount)
	{
		int32_t i = 0;
		const ::google::protobuf::FieldDescriptor **descriptors = new const ::google::protobuf::FieldDescriptor *[count];

		for(i = 0;i < this->_fieldCount;i++)
		{
			descriptors[i] = this->_descriptor->FindFieldByName(PQfname(this->_postgresqlRss, i));
			this->_fieldDescriptors[i] = nullptr;
		}
		delete []this->_fieldDescriptors;

		for(;i < count;i++)
			descriptors[i] = this->_descriptor->FindFieldByName(PQfname(this->_postgresqlRss, i));

		this->_fieldCount = count;
		this->_fieldDescriptors = descriptors;
	}
	else
	{
		if(this->_fieldCount <= 0)
		{
			this->_fieldCount = count;
			this->_fieldDescriptors = new const ::google::protobuf::FieldDescriptor *[count];
		}
		int32_t i = 0;
		for(;i < count;i++)
			this->_fieldDescriptors[i] = this->_descriptor->FindFieldByName(PQfname(this->_postgresqlRss, i));
		for(;i < this->_fieldCount;i++)
			this->_fieldDescriptors[i] = nullptr;
	}
}

/**copy message object dats**/
void Daterd::copyDat(::google::protobuf::Message *dat, const ::google::protobuf::FieldDescriptor *descriptor, char *value)
{
	::google::protobuf::FieldDescriptor::CppType type = descriptor->cpp_type();
	if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_STRING)
		dat->GetReflection()->SetString(dat, descriptor, value);
	else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_FLOAT)
		dat->GetReflection()->SetFloat(dat, descriptor, std::stof(value));
	else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE)
		dat->GetReflection()->SetDouble(dat, descriptor, std::stof(value));
	else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_INT32)
		dat->GetReflection()->SetInt32(dat, descriptor, std::stoi(value));
	else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_INT64)
		dat->GetReflection()->SetInt64(dat, descriptor, std::stol(value));
	else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_UINT32)
		dat->GetReflection()->SetUInt32(dat, descriptor, std::stoi(value));
	else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_UINT64)
		dat->GetReflection()->SetUInt64(dat, descriptor, std::stol(value));
}

/**init**/
void Dbers::init()
{
	Dbers::_dbers = new std::unordered_map<int32_t, struct _dber_t_ *>();
}

/**get datas**/
void Dbers::getDats(Daterd *dater, struct _dber_sock_t_ *sock, std::string str)
{
	if(sock->_dber->_type == _dbs_mysql_)
	{
		if(mysql_query(sock->_mysql, (char *)str.c_str()) == 0)
		{
			dater->bind(sock, mysql_store_result(sock->_mysql));
		}
		else
		{

		}
	}
	else
	{
		printf("------->%s\n", str.c_str());
	}
}

/**opt data**/
uint64_t Dbers::optDat(struct _dber_sock_t_ *sock, int32_t type, std::string str)
{
	uint64_t n = 0;
	if(sock->_dber->_type == _dbs_mysql_)
	{
		if(mysql_query(sock->_mysql, (char *)str.c_str()) == 0)
		{
			if(type == _dbs_add_)
			{
				n = mysql_insert_id(sock->_mysql);
				if(n <= 0)
					n = mysql_affected_rows(sock->_mysql);
			}
			else
			{
				n = mysql_affected_rows(sock->_mysql);
			}
		}
	}
	if(sock->_dber->_type == _dbs_postgresql_)
	{
		PGresult *rss = PQexec(sock->_postgresql, str.c_str());
		if(PQresultStatus(rss) == PGRES_COMMAND_OK)
		{
			PQclear(rss);
			n = 1;
		}
	}
	Dbers::releaseSock(sock);
	printf("------->type:%d, %s\n", type, str.c_str());
	return n;
}

/**add dber**/
void Dbers::addDber(int32_t id, int32_t type, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count)
{
	struct _dber_t_ *dber = nullptr;

	Dbers::_mtx.lock();

	dber = _maps_find_(Dbers::_dbers, id);
	if(dber == nullptr)
	{
		dber = Dbers::newDber(id, type, hname, port, uname, pwd, dbname, count);
		_maps_insert_(Dbers::_dbers, id, dber);

		for(int32_t i = 0;i < count;i++)
			Dbers::addSock(dber);
	}

	Dbers::_mtx.unlock();
}

/**new dber**/
struct _dber_t_ *Dbers::newDber(int32_t id, int32_t type, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count)
{
	struct _dber_t_ *dber = new struct _dber_t_();
	dber = new struct _dber_t_();
	dber->_id = id;
	dber->_type = type;
	dber->_count = count;
	dber->_hname = hname;
	dber->_port = port;
	dber->_uname = uname;
	dber->_pwd = pwd;
	dber->_dbname = dbname;
	return dber;
}

/**add sock**/
void Dbers::addSock(struct _dber_t_ *dber)
{
	struct _dber_sock_t_ *sock = new struct _dber_sock_t_();
	sock->_inUse = false;
	sock->_mysql = nullptr;
	sock->_postgresql = nullptr;
	sock->_dber = dber;
	sock->_prev = nullptr;
	sock->_next = nullptr;

	_nodes_add_(&Dbers::_head, &Dbers::_rear, sock);
}

/**get sock**/
struct _dber_sock_t_ *Dbers::getSock(int32_t id)
{
	struct _dber_sock_t_ *sock = nullptr;
	while(true)
	{
		Dbers::_mtx.lock();
		for(sock = Dbers::_head;sock != nullptr;sock = sock->_next)
		{
			if(sock->_dber->_id == id)
			{
				sock->_inUse = true;
				_nodes_remove_(&Dbers::_head, &Dbers::_rear, sock);
				break;
			}
		}
		Dbers::_mtx.unlock();
		if(sock == nullptr)
		{
			std::unique_lock<std::mutex> lck(Dbers::_wmtx);
			Dbers::_wcv.wait(lck);	
		}
		else
		{
			break;
		}
	}
	return sock;
}

/**release sock**/
void Dbers::releaseSock(struct _dber_sock_t_ *sock)
{
	bool flag = false;

	Dbers::_mtx.lock();
	if(sock->_inUse)
	{
		flag = true;
		sock->_inUse = false;
		_nodes_add_(&Dbers::_head, &Dbers::_rear, sock);
	}
	Dbers::_mtx.unlock();

	if(flag)
	{
		std::unique_lock<std::mutex> lck(Dbers::_wmtx);
		Dbers::_wcv.notify_one();
	}
}

/**connect sock**/
bool Dbers::connectSock(struct _dber_sock_t_ *sock)
{
	bool r = false;
	if(sock->_dber->_type == _dbs_mysql_)
	{
		if(sock->_mysql == nullptr)
		{
			sock->_mysql = mysql_init(nullptr);
			if(sock->_mysql != nullptr)
			{
				if(mysql_real_connect(sock->_mysql, (char *)sock->_dber->_hname.c_str(), (char *)sock->_dber->_uname.c_str(), (char *)sock->_dber->_pwd.c_str(), (char *)sock->_dber->_dbname.c_str(), sock->_dber->_port, NULL, 0) != nullptr)
				{
					char value = 1;
					mysql_options(sock->_mysql, MYSQL_OPT_RECONNECT, &value);
				
					mysql_query(sock->_mysql,"set names utf8");

					r = true;
				}
				else
				{
					mysql_close(sock->_mysql);
					sock->_mysql = nullptr;
				}
			}
		}
		else
		{
			if(mysql_ping(sock->_mysql) != 0)
			{
				mysql_close(sock->_mysql);
				sock->_mysql = nullptr;
			}
			else
			{
				r = true;
			}
		}
	}
	if(sock->_dber->_type == _dbs_postgresql_)
	{
		if(sock->_postgresql == nullptr)
		{
			char url[1024];
			int32_t n = snprintf(url, 1024, "postgresql://%s:%s@%s:%d/%s", sock->_dber->_uname.c_str(), sock->_dber->_pwd.c_str(), sock->_dber->_hname.c_str(), sock->_dber->_port, sock->_dber->_dbname.c_str());
			memset(url + n, 0, 1024 - n);

			sock->_postgresql = PQconnectdb(url);
			if(PQstatus(sock->_postgresql) == CONNECTION_OK)
			{
				r = true;
			}
			else
			{
				PQfinish(sock->_postgresql);
				sock->_postgresql = nullptr;
			}
		}
		else
		{
			PQreset(sock->_postgresql);
			r = true;
		}
	}
	if(sock->_dber->_type == _dbs_sqlite_)
	{
		if(sock->_sqlite3 != nullptr)
		{
			sqlite3_close(sock->_sqlite3);
			sock->_sqlite3 = nullptr;
		}
		if(sock->_sqlite3 == nullptr)
		{
			if(sqlite3_open((char *)sock->_dber->_hname.c_str(), &sock->_sqlite3) == 0)
			{
				r = true;
			}
			else
			{
				sqlite3_close(sock->_sqlite3);
				sock->_sqlite3 = nullptr;
			}
		}
	}
	return r;
}

/**check sock**/
struct _dber_sock_t_ *Dbers::checkSock(int32_t id)
{
	struct _dber_sock_t_ *sock = Dbers::getSock(id);
	if(sock != nullptr)
	{
		if(!Dbers::connectSock(sock))
		{
			Dbers::releaseSock(sock);
			sock = nullptr;
		}
	}
	return sock;
}

}