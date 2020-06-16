### 一、dbs工具概要：
    以Google protobuf为数据结构基础,对数据库实现线程池管理,实现对应的增、删、改、查等功能,目前支持mysql、progresql数据库,其余数据库后续增加。
    同时,对应的数据库C++版本的库文件和Google protobuf库文件自行安装。

### 二、数据库设计, 以mysql为例子:
    CREATE TABLE `s_user`  (
      `rid` int(20) NOT NULL COMMENT '用户标识',
      `name` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL COMMENT '用户名',
      `coin` int(11) NOT NULL COMMENT '用户金币',
      `gold` int(11) NOT NULL COMMENT '用户砖石',
      `rtime` int(11) NOT NULL COMMENT '注册时间',
      `ltime` int(11) NULL DEFAULT NULL COMMENT '最近登陆时间',
      `logoutime` int(11) NULL DEFAULT NULL COMMENT '最近登出时间',
      PRIMARY KEY (`rid`) USING BTREE,
    ) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_general_ci;

### 三、google protobuf协议设计,注意:协议名必须与数据库一致,不区分大小写,同时对应的协议属性值类型和属性名必须与数据库列名一直, 此地正确的协议名应该为S_user或者s_user,但为了测试对应的错误数据,改为User

    syntax = "proto3";

    message Cond
    {
      int32 				rid 		= 1;
    }

    message User
    {
      int32				rid 	= 1;
      string				name 	= 2;
      int32				coin 	= 3;
      int32 				gold 	= 4;
      int32				rtime 	= 5;
      int32				ltime 	= 6;
    }
  
### 四、需要在使用的地方引入:#include "dbs/dbers.h", 具体操作方法请看code\st\main.cc, 里面提供了所有api操作例子
   
