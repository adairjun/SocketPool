// Copyright 
// License
// Author: adairjun
// This is used to construct a socket connection pool 

#ifndef SOCLETPOOL_INCLUDE_SOCKET_CONNECTION_POOL_H
#define SOCKETPOOL_INCLUDE _SOCKET_CONNECTION_POOL_H

#include <string.h>
#include <mutex>
#include <map>
#include "socket_obj.h"

using std::string;
using std::mutex;
using std::unique_lock;
using std::multimap;

class ClientPool {
 public:
  ClientPool();
  virtual ~ClientPool();
  ClientPool(const ClientPool&) = delete;
  ClientPool& operator=(const ClientPool&) = delete;

  void Dump() const;

  bool Empty() const;
  /**
   * 从list当中选取一个连接
   * host和port是筛选的端口号
   */
  SocketObjPtr GetConnection(string host, unsigned port);

  /**
   * 释放特定的连接,就是把SocketObjPtr放回到list当中
   */
  bool ReleaseConnection(SocketObjPtr);

  /**
   * 构造函数创建poolsize个连接错误时候用来打印错误信息
   */
  string ErrorMessage() const;  

 private:
  mutex resource_mutex;

 public:
  //使用multimap来保存连接,而不用list
  multimap<string, SocketObjPtr> client_map;

 private:
  //这里是client连接的服务器的地址,所以除了clientPoolSize_,其他的都不能用client来命名
  string clientConnectHost_;
  unsigned clientConnectPort_;
  int clientConnectBacklog_;
  int clientPoolSize_;

  //错误信息
  string strErrorMessage_;
};

typedef boost::shared_ptr<ClientPool> ClientPoolPtr;
#endif /* SOCKETPOOL_INCLUD _SOCKET_CONNECTION_POOL_H */
