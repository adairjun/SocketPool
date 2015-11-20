/** 这个函数和unp_client和unp_server不同的地方在于unp_client和unp_server使用了select,这里我想使用epoll来做
  * 使用epoll不如直接使用封装了epoll的libevent
  */

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <Theron/Theron.h>
#include <gtest/gtest.h>
#include <event2/event.h>
#include "SocketPool/socket_connection_pool.h"

using std::cout;
using std::endl;

int main(int argc, char** argv) {
  return 0;
}
