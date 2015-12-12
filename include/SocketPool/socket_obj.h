// Copyright 
// License
// Author: adairjun
// This is used to construct a socket connection obj 

#ifndef SOCKETPOOL_INCLUDE_SOCKET_OBJ_H
#define SOCKETPOOL_INCLUDE_SOCKET_OBJ_H

#include <string>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <glog/logging.h>

using std::string;
using std::pair;
using std::make_pair;

class SocketObj;

// 使用shared_ptr来替换SocketObj*
typedef boost::shared_ptr<SocketObj>SocketObjPtr;

class SocketObj {
 public:
  // 虽然作为client的话.backlog用不到,但是这里还是要写入构造函数当中
  explicit SocketObj(string host, unsigned port, int backlog);
  explicit SocketObj(string host, unsigned port);
  explicit SocketObj(int sockFD);
  virtual ~SocketObj();
  SocketObj(const SocketObj&) = delete;
  SocketObj& operator=(const SocketObj&) = delete;
  void Dump() const;

  string ErrorMessage();
  
  /**
   * 设置套接字的阻塞模式,nonblock为true时是非阻塞模式
   */
  int SetNonBlock(bool nonblock); 

  /**
   * 由于sockaddr_in.sin_addr.s_addr类型是unsigned类型
   * 这个函数的作用就是把ip地址转换成s_addr能接受的类型
   * 方便构建sockaddr_in
   */
  unsigned TranslateAddress();
  
  /**
   * 封装的Bind函数
   */
  bool Bind();

  /**
   * 封装的listen函数
   */
  bool Listen();

  /**
   * 封装的accept函数,由于accept函数返回一个套接字,这里直接返回int
   */
  int Accept();
   
  /**
   * 封装的connect函数
   */
  bool Connect();

  /**
   * 封装的close函数
   */
  bool Close();

  /**
   * 获取套接字
   */
  int Get() const {
    return sockFD_;
  }
 
  /**
   * 封装的getpeername函数,返回远端地址和端口的map
   */
  pair<string, int> GetPeer();

  /**
   * 封装的getsockname函数,返回本地地址和端口的map
   */
  pair<string, int> GetSock();

 private:
  // 通过socket函数构建的套接字
  int sockFD_;
 
  string strHost_;
  unsigned iPort_;
  
  //本来需要一个sockaddr_in类型，来存储ip地址和端口号,但是在构造函数当中很难构造出一个完整的类型,总会报错incomplete type
  //那么就无法将sockaddr_in写进成员当中

  //用于listen函数的backlog_,决定内核为socket排队的最大连接个数
  //从配置文件当中读取
  int backlog_;

  //错误信息
  string strErrorMessage_;
};

#endif /* SOCKETPOOL_INCLUDE_SOCKET_OBJ_H */
