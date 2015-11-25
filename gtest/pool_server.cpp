#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <thread>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include "SocketPool/socket_connection_pool.h"

using std::cout;
using std::cerr;
using std::endl;

const string HOST = "127.0.0.1";
const unsigned PORT = 9999;

const int MAXLINE = 1024;
char buf[MAXLINE];

// 初始化全局的socket连接池
static SocketPoolPtr socketpool_ptr(new SocketPool);

typedef struct mybase {
  SocketObjPtr socketPtr;
  struct event_base* base;
}mybaseStruct;

void do_accept(evutil_socket_t listenfd, short event, void *arg);
void read_cb(struct bufferevent *bev, void *arg);
void write_cb(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, short event, void *arg);

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  //从连接池当中取出端口号为9999的连接
  SocketObjPtr listener = socketpool_ptr->GetConnection(true, HOST, PORT);
  //==========================================================
  //从这里开始写server代码
  int listenfd = listener->Get();
  //这里不用epoll了,而是使用封装了epoll的libevent
  //因为这个socket连接池的代码还不需要那么极致的性能
  struct event_base* base = event_base_new();
  //仅仅这一条语句就相当于 int efd = epoll_create(FDSIZE); struct epoll_event ev[EPOLLEVENTS]; 这两条语句
  //可见epoll的语法要比select简洁,而libevent的语法又比epoll简洁
  assert(base != NULL);
  mybaseStruct mbs;
  mbs.socketPtr = listener;
  mbs.base = base;
  struct event* listen_event = event_new(base, listenfd, EV_READ|EV_PERSIST, do_accept, (void*)&mbs);
  //仅仅这一条语句就相当于 struct epoll_event tmp; tmp.events = EPOLLIN; tmp.data.fd = listenfd;这三条语句,其中EV_READ就相当于EPOLLIN,
  //EV_PERSIST这个属性的话,如果不指定这个属性,回调函数被触发之后事件会被删除,这可是梦寐以求的特性啊,还记得在unp_server当中从客户端read之后要删除EPOLLIN事件吗,使用这个特性能简化掉那些代码,关键是能够避免忘记删除EPOLLIN事件产生的错误  
  //由于在回调函数中需要使用SocketObjPtr,这里我定义一个新的结构体mybase
  event_add(listen_event, NULL);
  //这一条语句就相当于 epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tmp); 

  //在unp_server当中,这里本来应该定义char buf[MAXLINE];我把它写成全局的
  //这里相比unp_server连while (true)的大循环都省了
  event_base_dispatch(base);

  //事件循环结束,清理bufferevent和event
  event_base_free(base);
  //==========================================================
  //==========================================================

  socketpool_ptr->ReleaseConnection(true, listener);
  return 0;
}

//当listenfd上有事件发生时,将调用回调函数do_accept
void do_accept(evutil_socket_t listenfd, short event, void *arg) {
  mybaseStruct* mbsPtr = (mybaseStruct*)arg;
  SocketObjPtr listener = mbsPtr->socketPtr;
  struct event_base* base = mbsPtr->base;
  evutil_socket_t fd = listener->Accept();  
  if (fd < 0) {
    cerr << "accept is error" << endl;
  } 
  //使用bufferevent,就是连带buffer一起管理的libevent,使用bufferevent来操作读写,不用使用read和write的系统调用
  //其实相当于一个高级一点的struct event_base* base = event_base_new();
  struct bufferevent* bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  //这里设置好当需要读写和错误发生的回调函数
  bufferevent_setcb(bev, read_cb, NULL, error_cb, arg);
  //还记得unp_server当中的删除EPOLLIN事件吗?我当时说的是也可以不删除,只要是read失败了再删除
  bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST); 
}

void read_cb(struct bufferevent *bev, void *arg) {
  int n;
  evutil_socket_t fd = bufferevent_getfd(bev);
  while (n=bufferevent_read(bev, buf, MAXLINE), n>0) {
    //回显给客户端
    bufferevent_write(bev, buf, n);
    //将buf写回到客户端的时候一定要清空buf,否则的话下次read的时候buf里面会存在客户端上次发送的残留
    memset(buf, 0, sizeof(buf));
  }  
}

void write_cb(struct bufferevent *bev, void *arg) { }

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
