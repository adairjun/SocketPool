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


int main(int argc, char** argv) {
  // 使用glog来打日志,除错
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = "../log";  
  SocketObj listener(HOST, PORT, BACKLOG);
  if (listener.Listen() != 0) {
    cerr << "Sorry, listen error!" << endl;
  }
  while (true) {
    SocketObjPtr sockPtr = listener.Accept();
    send(sockPtr->Get(), "worldhello", 10, 0);
    sockPtr->Close();
  }
  listener.Close();
  return 0;
}
