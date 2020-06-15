#ifndef __S_DBS_DBER_S_H__
#define __S_DBS_DBER_S_H__

#include <stdint.h>
#include <string.h>
#include <string>
#include <tuple>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

#include <google/protobuf/message.h>
#include <mysql.h>
#include <libpq-fe.h>

namespace dbs
{

/**mysql**/
#define _dbs_mysql_ 0
/**postgresql**/
#define _dbs_postgresql_ 1

/**gets**/
#define _dbs_gets_ 0
/**add**/
#define _dbs_add_ 1
/**update**/
#define _dbs_update_ 2
/**remove**/
#define _dbs_remove_ 3

/**cmd**/
class Cmd;
/**daterd**/
class Daterd;
/**dater**/
template <typename T>
class Dater;
/**dbers**/
class Dbers;

/**dber**/
struct _dber_t_
{
	int32_t 						_id				= -1;
	int32_t							_type 			= -1;
	int32_t							_count 			= 0;
	std::string 					_hname;
	int32_t							_port 			= -1;
	std::string 					_uname;
	std::string 					_pwd;
	std::string 					_dbname;
};

/**dber socket**/
struct _dber_sock_t_
{
	int32_t								_inUse 				= false;
	MYSQL 								*_mysql 			= nullptr;
	PGconn								*_postgresql 		= nullptr;
	struct _dber_t_ 					*_dber 				= nullptr;
	struct _dber_sock_t_ 				*_prev 				= nullptr;
	struct _dber_sock_t_ 				*_next 				= nullptr;	
};

/**maps find**/
template <typename K, typename V>
V _maps_find_(std::unordered_map<K, V> *maps, K k)
{
	auto itr = maps->find(k);
	if(itr != maps->end())
		return itr->second;
	return nullptr;
}

/**maps insert**/
template <typename K, typename V>
void _maps_insert_(std::unordered_map<K, V> *maps, K k, V v)
{
	maps->insert(std::pair<K, V>(k, v));
}

/**nodes add**/
template <typename T>
void _nodes_add_(T *head, T *rear, T v)
{
	v->_next = nullptr;
	if((*rear) == nullptr)
	{
		v->_prev = nullptr;
		(*head) = v;
	}
	else
	{
		v->_prev = (*rear);
		(*rear)->_next = v;
	}
	(*rear) = v;
}

/**nodes remove**/
template <typename T>
void _nodes_remove_(T *head, T *rear, T v)
{
	if(v->_prev == nullptr)
	{
		(*head) = v->_next;
		if((*head) != nullptr)
			(*head)->_prev = nullptr;
	}
	else
	{
		v->_prev->_next = v->_next;
	}
	if(v->_next == nullptr)
	{
		(*rear) = v->_prev;
		if((*rear) != nullptr)
			(*rear)->_next = nullptr;
	}
	else
	{
		v->_next->_prev = v->_prev;
	}
	v->_prev = nullptr;
	v->_next = nullptr;
}

/**cmd format append string**/
struct _cmd_format_append_string_t_
{
	static void append(std::string &s, struct _dber_sock_t_ *sock, std::string arg)
	{
		s.append("\'");
		if(sock->_dber->_type == _dbs_mysql_)
		{
			int32_t n = arg.length();

			char argBufs[n];
			mysql_real_escape_string(sock->_mysql, argBufs, arg.c_str(), n);
			
			s.append(argBufs);
		}
		else if(sock->_dber->_type == _dbs_postgresql_)
		{
			int32_t error;
			char bufs[255];
			PQescapeStringConn(sock->_postgresql, bufs, arg.c_str(), arg.length(), &error);

			s.append(bufs);
		}
		else
		{
			s.append(arg); 
		}
		s.append("\'");
	}
};

/**cmd format append arg**/
template <typename Arg>
struct _cmd_format_append_arg_t_
{
	static void append(std::string &s, struct _dber_sock_t_ *sock, Arg arg)
	{
		s.append(std::to_string(arg));
	}
};

/**cmd format append arg**/
template <>
struct _cmd_format_append_arg_t_<char *>
{
	static void append(std::string &s, struct _dber_sock_t_ *sock, char *arg)
	{
		_cmd_format_append_string_t_::append(s, sock, arg);
	}
};

/**cmd format append arg**/
template <>
struct _cmd_format_append_arg_t_<const char *>
{
	static void append(std::string &s, struct _dber_sock_t_ *sock, const char *arg)
	{
		_cmd_format_append_string_t_::append(s, sock, arg);
	}
};

/**cmd format append arg**/
template <>
struct _cmd_format_append_arg_t_<std::string>
{
	static void append(std::string &s, struct _dber_sock_t_ *sock, std::string arg)
	{
		_cmd_format_append_string_t_::append(s, sock, arg);
	}
};

/**cmd format append dat**/
struct _cmt_format_append_dat_t_
{
	static void append(std::string &s, struct _dber_sock_t_ *sock, const ::google::protobuf::Message *dat, const ::google::protobuf::FieldDescriptor *field)
	{
		if(field != nullptr)
		{
			::google::protobuf::FieldDescriptor::CppType type = field->cpp_type();
			if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_STRING)
			{
				std::string arg = dat->GetReflection()->GetString(*dat, field); _cmd_format_append_arg_t_<std::string>::append(s, sock, arg);
			}
			else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_FLOAT)
			{
				float arg = dat->GetReflection()->GetFloat(*dat, field); _cmd_format_append_arg_t_<float>::append(s, sock, arg);
			}
			else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE)
			{
				double arg = dat->GetReflection()->GetDouble(*dat, field); _cmd_format_append_arg_t_<double>::append(s, sock, arg);
			}
			else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_INT32)
			{
				int32_t arg = dat->GetReflection()->GetInt32(*dat, field); _cmd_format_append_arg_t_<int32_t>::append(s, sock, arg);
			}
			else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_INT64)
			{
				int64_t arg = dat->GetReflection()->GetInt64(*dat, field); _cmd_format_append_arg_t_<int64_t>::append(s, sock, arg);
			}
			else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_UINT32)
			{
				uint32_t arg = dat->GetReflection()->GetUInt32(*dat, field); _cmd_format_append_arg_t_<uint32_t>::append(s, sock, arg);
			}
			else if(type == ::google::protobuf::FieldDescriptor::CPPTYPE_UINT64)
			{
				uint64_t arg = dat->GetReflection()->GetUInt64(*dat, field); _cmd_format_append_arg_t_<uint64_t>::append(s, sock, arg);
			}
		}
	}
};

/**cmd format arg**/
template <typename Arg>
struct _cmd_format_arg_t_
{
	static void format(int32_t &i, std::string &s, struct _dber_sock_t_ *sock, const char *fmt, Arg arg)
	{
		int32_t m = 0;
		fmt += i;
		while(*fmt != '\0')
		{
			i++;
			if(*fmt == '{')
			{
				m = 1;
			}
			else if(*fmt == '}')
			{
				fmt++;
				_cmd_format_append_arg_t_<Arg>::append(s, sock, arg);
				break;
			}
			if(m == 0)
			{
				s.append(1, *fmt);
				fmt++;
			}
			else
			{
				fmt++;
			}
		}
	}
};

/**cmd format dat**/
template <typename T>
struct _cmd_format_dat_t_
{
	static std::string format(struct _dber_sock_t_ *sock, const char *fmt, T dat)
	{
		std::string s;

		int32_t m = 0;
		std::string param;

		while(*fmt != '\0')
		{
			if(*fmt == '{')
			{
				m = 1;
			}
			else if(*fmt == '}')
			{
				_cmt_format_append_dat_t_::append(s, sock, dat, dat->GetDescriptor()->FindFieldByLowercaseName(param));
		
				m = 0;
				param.clear();
			}
			else if(m == 1)
			{
				param.append(1, *fmt);
			}
			else if(m == 0)
			{
				s.append(1, *fmt);
			}
			fmt++;
		}

	return s;
	}
};

/**cmd format**/
template<typename Tuple, size_t N>
struct _cmd_format_t_
{
	static void format(int32_t &i, std::string &s, struct _dber_sock_t_ *sock, const char *fmt, const Tuple &tuple)
	{
		_cmd_format_t_<Tuple, N - 1>::format(i, s, sock, fmt, tuple);
		
		auto param = std::get<N - 1>(tuple);
		_cmd_format_arg_t_<decltype(param)>::format(i, s, sock, fmt, param);
	}
};

/**cmd format**/
template <typename Tuple>
struct _cmd_format_t_<Tuple, 1>
{
	static void format(int32_t &i, std::string &s, struct _dber_sock_t_ *sock, const char *fmt, const Tuple &tuple)
	{
		auto param = std::get<0>(tuple);
		_cmd_format_arg_t_<decltype(param)>::format(i, s, sock, fmt, param);
	}
};

/**cmd format**/
template <typename Tuple>
struct _cmd_format_t_<Tuple, 0>
{
	static void format(int32_t &i, std::string &s, struct _dber_sock_t_ *sock, const char *fmt, const Tuple &tuple)
	{
		 
	}
};

/**result**/
class Result
{
public:
	Result();
	~Result();
public:
	/**affected**/
	uint64_t affected();
	/**affected**/
	void setAffected(uint64_t affected);
	/**is error**/
	bool isError();
	/**error**/
	void error();
	/**str**/
	std::string str();
	/**str**/
	void setStr(std::string str);
protected:
	/**affected**/
	uint64_t 																		_affected 				= 0;
	/**error**/
	bool																			_error 					= false;
	/**str**/
	std::string 																	_str;
};

/**affected**/
inline uint64_t Result::affected() { return this->_affected; }
/**affected**/
inline void Result::setAffected(uint64_t affected) { this->_affected = affected; }
/**is error**/
inline bool Result::isError() { return this->_error; }
/**error**/
inline void Result::error() { this->_error = true; }
/**str**/
inline std::string Result::str() { return this->_str; }
/**str**/
inline void Result::setStr(std::string str) { this->_str = str; }

/**daterd**/
class Daterd:public Result
{
public:
	Daterd();
	~Daterd();
public:
	/**descriptor**/
	const ::google::protobuf::Descriptor *descriptor();
	/**bind mysql dbs**/
	virtual void bind(struct _dber_sock_t_ *sock, MYSQL_RES *mysqlRss) = 0;
	/**bind postgresql dbs**/
	virtual void bind(struct _dber_sock_t_ *sock, PGresult *postgresqlRss) = 0;
	/**close**/
	void close();
protected:
	/**init field from mysql**/
	void initFieldFromMysql();
	/**init field from postgresql**/
	void initFieldFromPostgresql();
	/**copy message object dats**/
	void copyDat(::google::protobuf::Message *dat, const ::google::protobuf::FieldDescriptor *descriptor, char *value);
protected:
	/**pb descriptor**/
	const ::google::protobuf::Descriptor 											*_descriptor  			= nullptr;
	/**pb type**/
	const ::google::protobuf::Message 												*_prototype  			= nullptr;
protected:
	/**dber socket**/
	struct _dber_sock_t_ 															*_sock 					= nullptr;
	/**mysql results**/
	MYSQL_RES 																		*_mysqlRss 				= nullptr;
	/**postgresql result i**/
	int32_t																			_postgresqlRssI 		= 0;
	/**postgresql result n**/
	int32_t																			_postgresqlRssN 		= 0;
	/**postgresql results**/
	PGresult 																		*_postgresqlRss 		= nullptr;
	/**field count**/
	int32_t 																		_fieldCount 			= 0;
	/**field descriptor**/
	const ::google::protobuf::FieldDescriptor 										**_fieldDescriptors		= nullptr;
};

/**descriptor**/
inline const ::google::protobuf::Descriptor *Daterd::descriptor() { return this->_descriptor; }

/**dater**/
template <typename T>
class Dater:public Daterd
{
public:
	Dater()
	{
		this->_descriptor = T::GetDescriptor();
		if(this->_descriptor != nullptr)
			this->_prototype = ::google::protobuf::MessageFactory::generated_factory()->GetPrototype(this->_descriptor);
	}
	~Dater()
	{

	}
public:
	/**bind mysql dbs**/
	void bind(struct _dber_sock_t_ *sock, MYSQL_RES *mysqlRss);
	/**bind postgresql dbs**/
	void bind(struct _dber_sock_t_ *sock, PGresult *postgresqlRss);
	/**get row data**/
	T *get();
};

/**bind mysql dbs**/
template <typename T>
inline void Dater<T>::bind(struct _dber_sock_t_ *sock, MYSQL_RES *mysqlRss)
{
	this->_sock = sock;
	this->_mysqlRss = mysqlRss;

	this->initFieldFromMysql();
}

/**bind postgresql dbs**/
template <typename T>
inline void Dater<T>::bind(struct _dber_sock_t_ *sock, PGresult *postgresqlRss)
{
	this->_sock = sock;
	this->_postgresqlRss = postgresqlRss;

	this->initFieldFromPostgresql();
}

/**get row data**/
template <typename T>
inline T *Dater<T>::get()
{
	::google::protobuf::Message *dat = nullptr;

	if(this->_sock != nullptr)
	{
		if(this->_sock->_dber->_type == _dbs_mysql_)
		{
			if(this->_mysqlRss != nullptr)
			{
				MYSQL_ROW row;
				if((row = mysql_fetch_row(this->_mysqlRss)) != nullptr)
				{
					dat = this->_prototype->New();
					for(int32_t i = 0;i < this->_fieldCount;i++)
					{
						if(this->_fieldDescriptors[i] != nullptr)
							this->copyDat(dat, this->_fieldDescriptors[i], row[i]);
					}
				}
			}
		}
		else if(this->_sock->_dber->_type == _dbs_postgresql_)
		{
			if(this->_postgresqlRss != nullptr)
			{
				if(this->_postgresqlRssI < this->_postgresqlRssN)
				{
					dat = this->_prototype->New();
					for(int32_t i = 0;i < this->_fieldCount;i++)
					{
						if(this->_fieldDescriptors[i] != nullptr)
							this->copyDat(dat, this->_fieldDescriptors[i], PQgetvalue(this->_postgresqlRss, this->_postgresqlRssI, i));
					}
					this->_postgresqlRssI++;
				}
			}
		}
	}

	return dynamic_cast<T *>(dat);
}

/**cmd**/
class Cmd
{
public:
	/**format**/
	template <typename ...Args>
	static std::string format(struct _dber_sock_t_ *sock, std::string str, Args... args);
	/**format**/
	template <typename T>
	static std::string formatd(struct _dber_sock_t_ *sock, std::string str, T args);
	/**format**/
	template <typename ...Args>
	static std::string formatGets(struct _dber_sock_t_ *sock, const ::google::protobuf::Descriptor *descriptor, std::string cond, Args... args);
	/**format**/
	static std::string formatGetsd(struct _dber_sock_t_ *sock, const ::google::protobuf::Descriptor *descriptor);
	/**format**/
	static std::string formatGetsdc(struct _dber_sock_t_ *sock, const ::google::protobuf::Descriptor *descriptor, ::google::protobuf::Message *cond);
	/**format**/
	static std::string formatAddgd(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat);
	/**format**/
	template <typename ...Args>
	static std::string formatUpdatedg(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat, std::string cond, Args... args);
	/**format**/
	static std::string formatUpdatedgc(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat, ::google::protobuf::Message *cond);
	/**format**/
	static std::string formatRemovegd(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat);
private:
	/**format**/
	static std::string formatUpdateHead(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat);
	/**format**/
	static std::string formatCond(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat);
};

/**format**/
template <typename ...Args>
inline std::string Cmd::format(struct _dber_sock_t_ *sock, std::string str, Args... args)
{
	std::string s;
	if(sizeof...(Args) > 0)
	{
		int32_t i = 0;
		const char *fmt = str.c_str();

		auto params = std::forward_as_tuple(args...);
		_cmd_format_t_<decltype(params), sizeof...(Args)>::format(i, s, sock, fmt, params);

		fmt += i;
		while(*fmt != '\0')
		{
			s.append(1, *fmt);
			fmt++;
		}
	}
	else
	{
		s = str;
	}
	if(s[s.length() - 1] != ';')
		s.append(";");
	return s;
}

/**format**/
template <typename T>
inline std::string Cmd::formatd(struct _dber_sock_t_ *sock, std::string str, T args)
{
	std::string s = _cmd_format_dat_t_<T>::format(sock, str.c_str(), args);
	if(s[s.length() - 1] != ';')
		s.append(";");
	return s;
}

/**format**/
template <typename ...Args>
inline std::string Cmd::formatGets(struct _dber_sock_t_ *sock, const ::google::protobuf::Descriptor *descriptor, std::string cond, Args... args)
{
	std::string s= Cmd::formatGetsd(sock, descriptor);
	if(cond.length() > 0)
	{
		s.append(" where ");
		s += Cmd::format(sock, cond, std::forward<Args>(args)...);
	}
	if(s[s.length() - 1] != ';')
		s.append(";");
	return s;
}

/**format**/
template <typename ...Args>
inline std::string Cmd::formatUpdatedg(struct _dber_sock_t_ *sock, ::google::protobuf::Message *dat, std::string cond, Args... args)
{
	std::string s = Cmd::formatUpdateHead(sock, dat);
	if(cond.length() > 0)
	{
		s.append(" where ");
		s += Cmd::format(sock, cond, std::forward<Args>(args)...);
	}
	if(s[s.length() - 1] != ';')
		s.append(";");
	return s;
}

/**dbers**/
class Dbers
{
public:
	/**init**/
	static void init();
	/**add dber**/
	static void addMysql(int32_t id, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count);
	/**add postgresql**/
	static void addPostgresql(int32_t id, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count);
	/**get datas**/
	template <typename ...Args>
	static void gets(Daterd *dater, int32_t id, std::string str, Args... args);
	/**get datas**/
	static void getsd(Daterd *dater, int32_t id, std::string str, ::google::protobuf::Message *cond);
	/**get datas**/
	template <typename ...Args>
	static void getsg(Daterd *dater, int32_t id, std::string cond, Args... args);
	/**get datas**/
	static void getsgd(Daterd *dater, int32_t id);
	/**get datas**/
	static void getsgdc(Daterd *dater, int32_t id, ::google::protobuf::Message *dat);
	/**add data**/
	template <typename ...Args>
	static Result *add(int32_t id, std::string str, Args... args);
	/**add data**/
	static Result *addd(int32_t id, std::string str, ::google::protobuf::Message *dat);
	/**add data**/
	static Result *addgd(int32_t id, ::google::protobuf::Message *dat);
	/**update data**/
	template <typename ...Args>
	static Result *update(int32_t id, std::string str, Args... args);
	/**update data**/
	template <typename ...Args>
	static Result *updatedg(int32_t id, ::google::protobuf::Message *dat, std::string cond, Args... args);
	/**update data**/
	template <typename ...Args>
	static Result *updatedgc(int32_t id, ::google::protobuf::Message *dat, ::google::protobuf::Message *cond);
	/**remove data**/
	template <typename ...Args>
	static Result *remove(int32_t id, std::string str, Args... args);
	/**remove data**/
	static Result *removegd(int32_t id,::google::protobuf::Message *dat);
	/**release**/
	static void release(struct _dber_sock_t_ *sock);
private:
	/**get datas**/
	static void getDats(Daterd *dater, struct _dber_sock_t_ *sock, std::string str);
	/**opt data**/
	static Result *optDat(struct _dber_sock_t_ *sock, int32_t type, std::string str);
private:
	/**add dber**/
	static void addDber(int32_t id, int32_t type, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count);
	/**new dber**/
	static struct _dber_t_ *newDber(int32_t id, int32_t type, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count);
	/**add sock**/
	static void addSock(struct _dber_t_ *dber);
	/**get sock**/
	static struct _dber_sock_t_ *getSock(int32_t id);
	/**release sock**/
	static void releaseSock(struct _dber_sock_t_ *sock);
	/**connect sock**/
	static bool connectSock(struct _dber_sock_t_ *sock);
	/**check sock**/
	static struct _dber_sock_t_ *checkSock(int32_t id);
private:
	/**mutex**/
	static std::mutex 														_mtx;
	/**wait mutex**/
	static std::mutex 														_wmtx;
	/**wait**/
	static std::condition_variable 											_wcv;
	/**dber socket**/
	static struct _dber_sock_t_ 											*_head;
	/**dber socket**/
	static struct _dber_sock_t_ 											*_rear;
	/**dbers**/
	static std::unordered_map<int32_t, struct _dber_t_ *>					*_dbers;
};

/**add dber**/
inline void Dbers::addMysql(int32_t id, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count)
{
	Dbers::addDber(id, _dbs_mysql_, hname, port, uname, pwd, dbname, count);
}

/**add postgresql**/
inline void Dbers::addPostgresql(int32_t id, std::string hname, int32_t port, std::string uname, std::string pwd, std::string dbname, int32_t count)
{
	Dbers::addDber(id, _dbs_postgresql_, hname, port, uname, pwd, dbname, count);
}

/**get datas**/
template <typename ...Args>
inline void Dbers::gets(Daterd *dater, int32_t id, std::string str, Args... args)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		Dbers::getDats(dater, sock, Cmd::format(sock, str, std::forward<Args>(args)...));
}

/**get datas**/
inline void Dbers::getsd(Daterd *dater, int32_t id, std::string str, ::google::protobuf::Message *dat)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		Dbers::getDats(dater, sock, Cmd::formatd(sock, str, dat));
}

/**get datas**/
template <typename ...Args>
inline void Dbers::getsg(Daterd *dater, int32_t id, std::string cond, Args... args)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		Dbers::getDats(dater, sock, Cmd::formatGets(sock, dater->descriptor(), cond, std::forward<Args>(args)...));
}

/**get datas**/
inline void Dbers::getsgd(Daterd *dater, int32_t id)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		Dbers::getDats(dater, sock, Cmd::formatGets(sock, dater->descriptor(), ""));
}

/**get datas**/
inline void Dbers::getsgdc(Daterd *dater, int32_t id, ::google::protobuf::Message *dat)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		Dbers::getDats(dater, sock, Cmd::formatGetsdc(sock, dater->descriptor(), dat));
}

/**add data**/
template <typename ...Args>
inline Result *Dbers::add(int32_t id, std::string str, Args... args)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_add_, Cmd::format(sock, str, std::forward<Args>(args)...));
	return nullptr;
}

/**add data**/
inline Result *Dbers::addd(int32_t id, std::string str, ::google::protobuf::Message *dat)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_add_, Cmd::formatd(sock, str, dat));
	return nullptr;
}

/**add data**/
inline Result *Dbers::addgd(int32_t id, ::google::protobuf::Message *dat)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_add_, Cmd::formatAddgd(sock, dat));
	return nullptr;
}

/**update data**/
template <typename ...Args>
inline Result *Dbers::update(int32_t id, std::string str, Args... args)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_update_, Cmd::format(sock, str, std::forward<Args>(args)...));
	return nullptr;
}

/**update data**/
template <typename ...Args>
inline Result *Dbers::updatedg(int32_t id, ::google::protobuf::Message *dat, std::string cond, Args... args)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_update_, Cmd::formatUpdatedg(sock, dat, cond, std::forward<Args>(args)...));
	return nullptr;
}

/**update data**/
template <typename ...Args>
inline Result *Dbers::updatedgc(int32_t id, ::google::protobuf::Message *dat, ::google::protobuf::Message *cond)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_update_, Cmd::formatUpdatedgc(sock, dat, cond));
	return nullptr;
}

/**remove data**/
template <typename ...Args>
inline Result *Dbers::remove(int32_t id, std::string str, Args... args)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_remove_, Cmd::format(sock, str, std::forward<Args>(args)...));
	return nullptr;
}

/**remove data**/
inline Result *Dbers::removegd(int32_t id,::google::protobuf::Message *dat)
{
	struct _dber_sock_t_ *sock = Dbers::checkSock(id);
	if(sock != nullptr)
		return Dbers::optDat(sock, _dbs_remove_, Cmd::formatRemovegd(sock, dat));
	return nullptr;
}

/**release**/
inline void Dbers::release(struct _dber_sock_t_ *sock)
{
	Dbers::releaseSock(sock);
}

}

#endif