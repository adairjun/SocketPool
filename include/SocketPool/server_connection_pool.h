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

class ServerPool {
 public:
  ServerPool();
  virtual ~ServerPool();
  ServerPool(const ServerPool&) = delete;
  ServerPool& operator=(const ServerPool&) = delete;

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
  //使用list来保存连接,而不用map
  //这里和DBPool不一样,DBPool是仅有一个list
  multimap<string, SocketObjPtr> server_map;

 private:
  string serverHost_;
  unsigned serverPort_;
  int serverBacklog_;  
  //从socket.xml或者socket.json当中读取max_connections
  int serverPoolSize_;

  //错误信息
  string strErrorMessage_;
};

typedef boost::shared_ptr<ServerPool> ServerPoolPtr;
#endif /* SOCKETPOOL_INCLUD _SOCKET_CONNECTION_POOL_H */
