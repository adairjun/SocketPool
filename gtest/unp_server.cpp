// UNPv1 书当中使用的是select来进行IO复用,这里我使用epoll来做IO复用
#include <iostream>
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include "SocketPool/socket_obj.h"

#include <sys/epoll.h>

using std::cout;
using std::cerr;
using std::endl;

// HOST留空说明设定为INADDR_ANY
const string HOST = "";
const unsigned PORT = 9999;
const int BACKLOG = 10;

const int FDSIZE = 10;
const int EPOLLEVENTS = 8;

const int MAXLINE = 1024;
void str_echo(int sockfd) {
  ssize_t n;
  char buf[MAXLINE]; 
  
  while ((n=read(sockfd, buf, MAXLINE)) > 0) { 
    write(sockfd, buf, n);
  }
  if (n<0) {
    cerr << "str_echo: read error" << endl;
  }
}

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  SocketObj listener(HOST, PORT, BACKLOG);
  if (listener.Listen() != 0) {
    cerr << "Sorry, listen error!" << endl;
  }
  int listenfd = listener.Get();
  //创建一个epoll句柄
  int efd = epoll_create(FDSIZE);
  //初始化一个事件  
  struct epoll_event ev[EPOLLEVENTS];
  
  //这里注册epoll事件,listener.Get()只注册读入的事件,不注册写出的事件,因为epoll只需要监视读入的事件,因为listener只会读入,不会写出
  //EPOLLIN就是读入事件
  struct epoll_event tmp;
  tmp.events = EPOLLIN;
  tmp.data.fd = listenfd;   //listener.Get()得到socket描述符
  //注册一个事件
  epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tmp);   //EPOLL_CTL_ADD是加入
  int ret; 

  while (true) {
    //epoll监视的只是连接而已
cout << " eeeeeeeeeeeeeeee" << endl;
    ret = epoll_wait(efd, ev, EPOLLEVENTS, -1);        //-1这个位置设置的是一个超时值,设为-1表示永久阻塞
  cout << "ret ======" << ret << endl;
    int ev_fd;
    for (int i=0; i<ret; ++i) {
      ev_fd = ev[i].data.fd;
      //epoll只需要监视读入的,不需要监视写出的
  cout << "4444444444444444444444444" << endl;
        //说明有读入,这里要判断一下是从STDIN_FILENO读入还是从socket读入
        if (ev_fd == listenfd) {
  cout << "5555555555555555555" << endl;
//=============================================================
//struct sockaddr_in clientaddr;
//socklen_t clilen = sizeof(clientaddr);
//int connfd = accept(listenfd, (sockaddr*)&clientaddr, &clilen);
//tmp.events = EPOLLIN;
//tmp.data.fd = connfd;
//epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tmp);   //EPOLL_CTL_ADD是加入
//cout <<"6666666666666666666" << endl;
//cout << "sockPtr->Get() " << connfd << endl;  
//============================================================
          //说明listener有事件发生,那么这个时候就要使用accept来获取连接了
          SocketObjPtr sockPtr = listener.Accept();
          int sockfd = sockPtr->Get(); //sockPtr->Get()得到socket描述符
          //从accept得到了与客户端的连接之后,也要把连接放入epoll的监听当中去
          //这比select的用法可是简单多了
          tmp.events = EPOLLIN;
          tmp.data.fd = sockfd;       
          //注册一个事件
          epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &tmp);   //EPOLL_CTL_ADD是加入
  cout <<"6666666666666666666" << endl;
  cout << "sockfd " << sockfd << endl;  
        } else if (ev[i].events & EPOLLIN) {
  cout << "777777777777777777777" << endl;
          //这里可不用像客户端的epoll一样用if来判断一下ev_fd等于哪个连接
          //因为根本没这个必要,反正除了listener之外一定就是客户端连接了,所以ev_fd一定是客户端连接
          //至于这个连接是属于哪个客户端,根本不需要关心了
          str_echo(ev_fd);
        }
  cout << "88888888888888888" << endl;
  cout << "efd ====" << efd << endl;
    }
  }
  close(efd);
  listener.Close();
  return 0;
}
