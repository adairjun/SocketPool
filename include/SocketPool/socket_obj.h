// Copyright 
// License
// Author: adairjun
// This is used to construct a socket connection obj 

#ifndef SOCKETPOOL_INCLUDE_SOCKET_OBJ_H
#define SOCKETPOOL_INCLUDE_SOCKET_OBJ_H

#include <string>
#include <boost/shared_ptr.hpp>

using std::string;

class SocketObj;

// 使用shared_ptr来替换SocketObj*
typedef boost::shared_ptr<SocketObj>SocketObjPtr;

class SocketObj {
 public:
  // 虽然作为client的话.backlog用不到,但是这里还是要写入构造函数当中
  explicit SocketObj(string host, unsigned port, int backlog);
  explicit SocketObj(int sockFD);
  virtual ~SocketObj();
  void Dump() const;

  // 由于sockaddr_in.sin_addr.s_addr类型是unsigned类型
  // 这个函数的作用就是把ip地址转换成s_addr能接受的类型
  // 方便构建sockaddr_in
  unsigned TranslateAddress();
  
  // 封装的Bind函数
  int Bind();

  //封装的listen函数
  int Listen();

  //封装的accept函数,由于accept函数返回一个套接字,这里再做一层封装,根据返回的套接字构建SocketObj对象 
  SocketObjPtr Accept();
   
  //封装的connect函数  
  int Connect();

  //封装的close函数
  int Close();

  //获取套接字
  int Get() const {
    return sockFD_;
  }

 private:
  // 通过socket函数构建的套接字
  int sockFD_;
 
  struct sockaddr_in sAddr_; 
  string strHost_;
  unsigned iPort_;
  
  //用于listen函数的backlog_,决定内核为socket排队的最大连接个数
  //从配置文件当中读取
  int backlog_;
};

#endif /* SOCKETPOOL_INCLUDE_SOCKET_OBJ_H */
