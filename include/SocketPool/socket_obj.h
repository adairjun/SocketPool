// Copyright 
// License
// Author: adairjun
// This is used to construct a socket connection obj 


#ifndef SOCKETPOOL_INCLUDE_SOCKET_OBJ_H
#define SOCKETPOOL_INCLUDE_SOCKET_OBJ_H

#include <boost/shared_ptr.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#ifdef __cplusplus
}
#endif

class SocketObj {
 public:
  SocketObj();
  virtual ~SocketObj();
 private:

};


// 使用shared_ptr来替换SocketObj*
typedef boost::shared_ptr<SocketObj>SocketObjPtr;
#endif /* SOCKETPOOL_INCLUDE_SOCKET_OBJ_H */

