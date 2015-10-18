// Copyright 
// License
// Author: adairjun
// This is used to construct a socket connection obj 


#ifndef SOCKETPOOL_INCLUDE_SOCKET_OBJ_H
#define SOCKETPOOL_INCLUDE_SOCKET_OBJ_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class SocketObj {
 public:
  SocketObj();
  virtual ~MysqlObj();
 private:

}


// 使用shared_ptr来替换SocketObj*
typedef boost::shared_ptr<SocketObj>SocketObjPtr;
#endif /* SOCKETPOOL_INCLUDE_SOCKET_OBJ_H */

