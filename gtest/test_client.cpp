#include <iostream>
#include <gtest/gtest.h>
#include "SocketPool/socket_obj.h"

using std::cout;
using std::cerr;
using std::endl;

const string HOST = "127.0.0.1";
const unsigned PORT = 9999;
// 由于是客户端不需要BACKLOG
const int BACKLOG = 0;

int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  SocketObj sock(HOST, PORT, BACKLOG);
  if (sock.Connect() !=0 ) {
    cerr << "Connect error" << endl;
  }
  int n;
  char recvline[40];
  while ((n=read(sock.Get(), recvline, 40)) > 0) {
    recvline[n] = 0;
    if (fputs(recvline, stdout) == EOF) {
      cerr << "fputs error" << endl;
    }
  }
  sock.Close();
  return 0;
}
