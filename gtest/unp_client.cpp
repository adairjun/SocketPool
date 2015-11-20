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

  tmp.events = EPOLLIN;
  tmp.data.fd = sockfd;      //sockfd 也是一个文件描述符
  epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &tmp);   //EPOLL_CTL_ADD是加入

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
        } 
        if (ev_fd == sockfd) { 
          //说明sockfd可用,那么就需要从sockfd读入数据
          if (read(sockfd, recvline, MAXLINE) == 0) {
            LOG(ERROR) << "read error" << endl;
          }          
          //从sockfd读入了,那么就准备向标准输出写数据
          fputs(recvline, stdout);
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
