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

void read_stdin(int fd, short events, void* arg); 
void read_socket(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, short event, void *arg);

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  //从连接池当中取出端口号为9999的连接
  if (!clientpool_ptr->Empty()) {
    SocketObjPtr clienter = clientpool_ptr->GetConnection(HOST, PORT);
    //========================================================== //从这里开始写client代码
    int clientfd = clienter->Get();
    struct event_base* base = event_base_new();
    assert(base != NULL);

    //bufferevent监听clientfd
    struct bufferevent* bev_socket = bufferevent_socket_new(base, clientfd, BEV_OPT_CLOSE_ON_FREE);
    //read_socket从socket读入, write_stdout写出到标准输出
    //所以这里需要传入参数clientfd
    bufferevent_setcb(bev_socket, read_socket, NULL, error_cb, (void*)&clientfd);
    //还记得unp_server当中的删除EPOLLIN事件吗?我当时说的是也可以不删除,只要是read失败了再删除
    bufferevent_enable(bev_socket, EV_READ|EV_PERSIST); 

    //监听终端输入事件
    struct event* ev_cmd = event_new(base, STDIN_FILENO, EV_READ|EV_PERSIST, read_stdin, (void*)bev_socket); 
    event_add(ev_cmd, NULL);

    event_base_dispatch(base);

    //事件循环结束,清理bufferevent和event
    bufferevent_free(bev_socket);
    event_base_free(base);
    //==========================================================

    clientpool_ptr->ReleaseConnection(clienter);
  } else {
    cerr << "Sorry, pool is empty!" << endl;  
  }
  return 0;
}

void read_stdin(int fd, short events, void* arg) {
  int ret = read(fd, sendline, MAXLINE);
  struct bufferevent* bev = (struct bufferevent*) arg;
  //write(clientfd, sendline, strlen(sendline));
  bufferevent_write(bev, sendline, strlen(sendline));
  memset(sendline, 0, sizeof(sendline));
}

void read_socket(struct bufferevent *bev, void *arg) {
  int len = bufferevent_read(bev, recvline, sizeof(recvline));
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
