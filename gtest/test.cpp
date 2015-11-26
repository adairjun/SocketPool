#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <thread>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "SocketPool/server_connection_pool.h"

using std::cout;
using std::cerr;
using std::endl;

const string HOST = "127.0.0.1";
const unsigned PORT = 9999;

const int MAXLINE = 1024;
char buf[MAXLINE];

// 初始化全局的socket连接池
static ServerPoolPtr serverpool_ptr(new ServerPool);

typedef struct mybase {
  SocketObjPtr socketPtr;
  struct event_base* base;
}mybaseStruct;

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  //从连接池当中取出端口号为9999的连接
  SocketObjPtr listener = serverpool_ptr->GetConnection(HOST, PORT);
  //==========================================================
  //==========================================================

  serverpool_ptr->ReleaseConnection(listener);
  return 0;
}

