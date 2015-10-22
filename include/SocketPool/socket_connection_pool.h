// Copyright 
// License
// Author: adairjun
// This is used to construct a socket connection pool 

#ifndef SOCLETPOOL_INCLUDE_SOCKET_CONNECTION_POOL_H
#define SOCKETPOOL_INCLUDE _SOCKET_CONNECTION_POOL_H

#include "socket_obj.h"
#include <list>

class SocketPool {
 public:
  SocketPool();
  virtual SocketPool();
  SocketPool(const SocketPool&) = delete;
  SocketPool& operator=(const SocketPool&) = delete;

 private:

};







typedef boost::shared_ptr<SocketPool> SocketPoolPtr;
#endif /* SOCKETPOOL_INCLUD _SOCKET_CONNECTION_POOL_H */
