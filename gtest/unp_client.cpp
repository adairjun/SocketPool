#include <iostream>
#include <gtest/gtest.h>
#include "SocketPool/socket_obj.h"

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

using std::cout;
using std::cerr;
using std::endl;

const string HOST = "127.0.0.1";
const unsigned PORT = 9999;
// 由于是客户端不需要BACKLOG
const int BACKLOG = 0;

const int MAXLINE = 256;
void str_cli(FILE *fp, int sockfd) {
  char sendline[MAXLINE];
  char recvline[MAXLINE];
  //如果当服务器进程被kill掉了,那么服务器会发送一个FIN,但是这个时候客户端却阻塞在了fgets函数上面,没有收到sockfd的FIN,无法响应ACK是当然的,ACK先不谈,这里就是客户端根本就不知道服务器已经关闭了,这就导致客户端还是会给服务器发数据,使用select来避免
  int maxfdpl;
  fd_set rset;
  FD_ZERO(&rset);
  //while (fgets(sendline, MAXLINE, fp) != NULL) {
  while(true) {
    FD_SET(fileno(fp), &rset);
    FD_SET(sockfd, &rset);
    maxfdpl = fileno(fp) > sockfd ? fileno(fp) : sockfd + 1;
    select(maxfdpl, &rset, NULL, NULL, NULL);
    if (FD_ISSET(sockfd, &rset)) {
      if(read(sockfd, recvline, MAXLINE) == 0) {
        LOG(ERROR) << "server terminated" << endl;
        cerr << "server terminated" << endl;
      }
      fputs(recvline, stdout);
      LOG(INFO) << "client send message from server: " << sendline << endl;
    } 
    if (FD_ISSET(fileno(fp), &rset)) {
      if (fgets(sendline, MAXLINE, fp)==NULL) {
        return;
      }
      write(sockfd, sendline, strlen(sendline));
      LOG(INFO) << "client receive message from server: " << recvline << endl;
    }
  }
}

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
