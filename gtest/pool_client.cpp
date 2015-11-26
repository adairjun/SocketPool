#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <thread>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "SocketPool/client_connection_pool.h"

using std::cout;
using std::cerr;
using std::endl;

const string HOST = "127.0.0.1";
const unsigned PORT = 9999;

const int MAXLINE = 1024;
char sendline[MAXLINE];
char recvline[MAXLINE];

// 初始化全局的socket连接池
static ClientPoolPtr clientpool_ptr(new ClientPool);

void read_stdin(struct bufferevent *bev, void *arg);
void read_socket(struct bufferevent *bev, void *arg);
void write_socket(struct bufferevent *bev, void *arg);
void write_stdout(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, short event, void *arg);

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  //从连接池当中取出端口号为9999的连接
  SocketObjPtr clienter = clientpool_ptr->GetConnection(HOST, PORT);
  //==========================================================
  //从这里开始写client代码
  int clientfd = clienter->Get();
  struct event_base* base = event_base_new();
  assert(base != NULL);
  //直接用bufferevent,不用libevent
  //首先bufferevent监听标准输入
  struct bufferevent* bev_stdin = bufferevent_socket_new(base, STDIN_FILENO, BEV_OPT_CLOSE_ON_FREE);
  //这里设置好当需要读写和错误发生的回调函数
  //read_stdin从stdin读入,write_socket向socket写出
  //所以这里需要传入参数clientfd
  bufferevent_setcb(bev_stdin, read_stdin, write_socket, error_cb, (void*)&clientfd);
  //还记得unp_server当中的删除EPOLLIN事件吗?我当时说的是也可以不删除,只要是read失败了再删除
  bufferevent_enable(bev_stdin, EV_READ|EV_WRITE|EV_PERSIST); 

  //bufferevent监听clientfd
  struct bufferevent* bev_socket = bufferevent_socket_new(base, clientfd, BEV_OPT_CLOSE_ON_FREE);
  //read_socket从socket读入, write_stdout写出到标准输出
  //所以这里需要传入参数clientfd
  bufferevent_setcb(bev_socket, read_socket, write_stdout, error_cb, (void*)&clientfd);
  //还记得unp_server当中的删除EPOLLIN事件吗?我当时说的是也可以不删除,只要是read失败了再删除
  bufferevent_enable(bev_socket, EV_READ|EV_WRITE|EV_PERSIST); 

  event_base_dispatch(base);

  //事件循环结束,清理bufferevent和event
  bufferevent_free(bev_stdin);
  bufferevent_free(bev_socket);
  event_base_free(base);
  //==========================================================

  clientpool_ptr->ReleaseConnection(clienter);
  return 0;
}

void read_stdin(struct bufferevent *bev, void *arg) {
  int n;
  evutil_socket_t fd = bufferevent_getfd(bev);
  //将标准输入读入到sendline缓冲区
  while (n=bufferevent_read(bev, sendline, MAXLINE), n>0) {
  }  
}

void write_socket(struct bufferevent *bev, void *arg) { 
  int* clientfdPtr = (int*)arg;
  int clientfd = *clientfdPtr;
  //int n=bufferevent_write(/*这个不能是bev*/, sendline, sizeof(sendline));
  write(clientfd, sendline, strlen(sendline));
  memset(sendline, 0, sizeof(sendline));
  //n要大于0 
}

void read_socket(struct bufferevent *bev, void *arg) {
  int n;
  evutil_socket_t fd = bufferevent_getfd(bev);
  //将socket读入到recvline缓冲区
  while (n=bufferevent_read(bev, recvline, MAXLINE), n>0) {
  }  
}

void write_stdout(struct bufferevent *bev, void *arg) { 
  //int n=bufferevent_write(, sendline, sizeof(sendline));
  //不能用bufferevent_write输出,因为不知道bev
  fputs(recvline, stdout);
  //把recvline打印到屏幕上之后,需要清空recvline,否则下次打印到屏幕的时候会有本地recvline的残留
  memset(recvline, 0, sizeof(recvline));
}

//和unp_server的策略一样,只要是有错误就删除bufferevent事件
void error_cb(struct bufferevent *bev, short event, void *arg) {
  evutil_socket_t fd = bufferevent_getfd(bev);
  if (event & BEV_EVENT_TIMEOUT) {
    LOG(ERROR) << "Time out\n"; 
  } else if (event & BEV_EVENT_EOF) {
    LOG(ERROR) << "Client closed\n";
  }
  bufferevent_free(bev); 
}
