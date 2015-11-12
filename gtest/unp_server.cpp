#include <iostream>
#include <gtest/gtest.h>
#include <arpa/inet.h>
#include "SocketPool/socket_obj.h"

using std::cout;
using std::cerr;
using std::endl;

// HOST留空说明设定为INADDR_ANY
const string HOST = "";
const unsigned PORT = 9999;
const int BACKLOG = 10;

const int MAXLINE = 256;
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
  int maxfd = listener.Get();
  int maxi = -1;
  //使用智能指针的数组
  SocketObjPtr client[FD_SETSIZE];
  fd_set rset, allset;
  FD_ZERO(&allset);
  FD_SET(listener.Get(), &allset);
  int nready;
  while (true) {
    rset = allset;
    nready = select(maxfd+1, &rset, NULL, NULL, NULL);
    if (FD_ISSET(listener.Get(), &rset)) {
      SocketObjPtr sockPtr = listener.Accept();
      int i = 0;
      for (i = 0; i< FD_SETSIZE; ++i) {
        if (client[i] == NULL) {
          client[i] = sockPtr;
          break;
        }
      } 
      FD_SET(sockPtr->Get(), &allset);
      maxfd = sockPtr->Get() > maxfd ? sockPtr->Get() : maxfd;
      maxi = i > maxi ? i : maxi;
      if (--nready <= 0) { 
        continue;
      } 
    } 
    for (int i = 0; i<= maxi; ++i) {
      int sockfd = client[i]->Get();
      if (sockfd < 0) {
        continue;
      }
      if (FD_ISSET(sockfd, &rset)) {
        ssize_t n;
        char buf[MAXLINE];
        if ((n=read(sockfd, buf, MAXLINE)) == 0) {
          client[i]->Close();
          FD_CLR(sockfd, &allset);
        } else {
          write(sockfd, buf, n);
        }
      }  
      if (--nready <= 0) {
        break;
      }
    }
  }
  listener.Close();
  return 0;
}
