// UNPv1 书当中使用的是select来进行IO复用,这里我使用epoll来做IO复用
#include <iostream>
#include <gtest/gtest.h>
#include "SocketPool/socket_obj.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;

const string HOST = "127.0.0.1";
const unsigned PORT = 9999;
// 由于是客户端不需要BACKLOG
const int BACKLOG = 0;

const int MAXLINE = 1024;

const int FDSIZE = 10;
const int EPOLLEVENTS = 8;

void str_cli(int sockfd) {
  //创建一个epoll句柄
  int efd = epoll_create(FDSIZE);
  //初始化一个事件  
  struct epoll_event ev[EPOLLEVENTS];
  
  //这里注册epoll事件,STDIN_FILENO和sockfd都只注册读入的事件,不注册写出的事件,因为epoll只需要监视读入的事件
  //EPOLLIN就是读入事件
  struct epoll_event tmp;
  tmp.events = EPOLLIN;
  tmp.data.fd = STDIN_FILENO;      //STDIN_FILENO 也是一个文件描述符
  //注册一个事件
  epoll_ctl(efd, EPOLL_CTL_ADD, STDIN_FILENO, &tmp);   //EPOLL_CTL_ADD是加入

// 切记,这里不能让sockfd一直都被epoll监视EPOLLIN状态,
// 因为如果这样的话,当服务器是用Ctrl+C终止的时候,服务器由于四路挥手,将会向sockfd发送一个FIN,
// 这将被epoll捕获为EPOLLIN状态,将触发从sockfd当中read的这个函数,
// 但是由于根本不可能从sockfd当中read出任何东西,将打出日志"read error",并重新进入while(true)这个大循环,
// 当然这个时候又一次被epoll捕获到了,结果就是一直不停地打印出日志"read error"
// 所以绝对不能在把sockfd的EPOLLIN的监听一直打开着
//  tmp.events = EPOLLIN;
//  tmp.data.fd = sockfd;      //sockfd 也是一个文件描述符
//  epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &tmp);   //EPOLL_CTL_ADD是加入
// 正确的做法是在判断从stdin读入并往sockfd写入之后,再用add加入,判断从sockfd读入之后,使用del删除

  char sendline[MAXLINE];
  char recvline[MAXLINE];
  int ret;
  while (true) {
    ret = epoll_wait(efd, ev, EPOLLEVENTS, -1);        //-1这个位置设置的是一个超时值,设为-1表示永久阻塞
    int ev_fd;
    for(int i=0; i<ret; ++i) {
      ev_fd = ev[i].data.fd;
      //epoll只需要监视读入的,不需要监视写出的
      if (ev[i].events & EPOLLIN) {
        //说明有读入,这里要判断一下是从STDIN_FILENO读入还是从socket读入
        if (ev_fd == STDIN_FILENO) {
          //说明STDIN_FILENO可用,那么现在就从STDIN_FILENO读取数据到sendline
          if (fgets(sendline, MAXLINE, stdin) == NULL) {
            LOG(ERROR) << "fgets error" << endl;
          }         
            //从STDIN_FILENO读取完成后,就准备向socket写数据 
          write(sockfd, sendline, strlen(sendline));
          //这个时候才使用epoll监听sockfd的EPOLLIN
          tmp.events = EPOLLIN;
          tmp.data.fd = sockfd;   
          epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &tmp);   //EPOLL_CTL_ADD是加入
        } 
        if (ev_fd == sockfd) { 
          //说明sockfd可用,那么就需要从sockfd读入数据
          if (read(sockfd, recvline, MAXLINE) == 0) {
            LOG(ERROR) << "read error" << endl;
          }          
          //从sockfd读入之后,马上删除sockfd的EPOLLIN
          tmp.events = EPOLLIN;
          tmp.data.fd = sockfd;   
          epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, &tmp);   //EPOLL_CTL_DEL是删除
          //从sockfd读入了,那么就准备向标准输出写数据
          fputs(recvline, stdout);
          //把recvline打印到屏幕上之后,需要清空recvline,否则下次打印到屏幕的时候会有本地recvline的残留
          memset(recvline, 0, sizeof(recvline));
        }
      } 
    }
  }
  close(efd);
}

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  SocketObj sock(HOST, PORT, BACKLOG);
  if (sock.Connect()<0) {
    cerr << "Connect error" << endl;
  }
  str_cli(sock.Get());
  sock.Close();
  return 0;
}
