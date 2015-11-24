// Copyright 
// License
// Author: adairjun
// This is used to construct a socket connection pool 

#ifndef SOCLETPOOL_INCLUDE_SOCKET_CONNECTION_POOL_H
#define SOCKETPOOL_INCLUDE _SOCKET_CONNECTION_POOL_H

#include <string.h>
#include <mutex>
#include <list>
#include "socket_obj.h"

using std::string;
using std::mutex;
using std::unique_lock;
using std::list;

class SocketPool {
 public:
  SocketPool();
  virtual ~SocketPool();
  SocketPool(const SocketPool&) = delete;
  SocketPool& operator=(const SocketPool&) = delete;
  /**
   * 从list当中选取一个连接,如果传入true,那么就从server_list当中选一个连接
   * host和port是筛选的端口号
   */
  SocketObjPtr GetConnection(bool server, string host, unsigned port);

  /**
   * 释放特定的连接,就是把SocketObjPtr放回到list当中
   * 第一个参数指定的是这个SocketObjPtr是server或者是client
   */
  int ReleaseConnection(bool server, SocketObjPtr);

  /**
   * 构造函数创建poolsize个连接错误时候用来打印错误信息
   */
  string ErrorMessage() const;  

 private:
  mutex resource_mutex;

 public:
  //使用list来保存连接,而不用map
  //这里和DBPool不一样,DBPool是仅有一个list
  list<SocketObjPtr> server_list;
  list<SocketObjPtr> client_list;

 private:
  string serverHost_;
  unsigned serverPort_;
  int serverBacklog_;  
  //从socket.xml或者socket.json当中读取max_connections
  int serverPoolSize_;

  //这里是client连接的服务器的地址,所以除了clientPoolSize_,其他的都不能用client来命名
  string clientConnectHost_;
  unsigned clientConnectPort_;
  int clientConnectBacklog_;
  int clientPoolSize_;

  //错误信息
  string strErrorMessage_;
};

typedef boost::shared_ptr<SocketPool> SocketPoolPtr;
#endif /* SOCKETPOOL_INCLUD _SOCKET_CONNECTION_POOL_H */
