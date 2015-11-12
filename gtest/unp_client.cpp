#include <iostream>
#include <gtest/gtest.h>
#include "SocketPool/socket_obj.h"

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using std::cout;
using std::cerr;
using std::endl;

const int MAXLINE = 256;

void str_cli(FILE *fp, int sockfd) {
  char sendline[MAXLINE];
  char recvline[MAXLINE];
  while (fgets(sendline, MAXLINE, fp) != NULL) {
    LOG(INFO) << "client send message from server: " << sendline << endl;
    write(sockfd, sendline, strlen(sendline));
    if(read(sockfd, recvline, MAXLINE) == 0) {
      LOG(ERROR) << "server terminated" << endl;
      cerr << "server terminated" << endl;
    }
    fputs(recvline, stdout);
    LOG(INFO) << "client receive message from server: " << recvline << endl;
  }
}

const string HOST = "127.0.0.1";
const unsigned PORT = 9999;
// 由于是客户端不需要BACKLOG
const int BACKLOG = 0;

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  SocketObj sock(HOST, PORT, BACKLOG);
  if (sock.Connect()<0) {
    cerr << "Connect error" << endl;
  }
  str_cli(stdin, sock.Get());
  sock.Close();
  return 0;
}
