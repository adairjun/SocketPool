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
  
  while((n=read(sockfd, buf, MAXLINE)) > 0) { 
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
  pid_t childpid;
  while (true) {
    SocketObjPtr sockPtr = listener.Accept();
    str_echo(sockPtr->Get());
    sockPtr->Close(); 
  }
  listener.Close();
  return 0;
}
