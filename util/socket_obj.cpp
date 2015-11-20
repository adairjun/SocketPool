#include "SocketPool/socket_obj.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

SocketObj::SocketObj(string host, unsigned port, int backlog)
    : strHost_(host),
      iPort_(port),
      backlog_(backlog) { 
    LOG(INFO) << "consutor function HOST:[" << host << "]" 
              << " PORT:[" << port << "]" 
              << " BACKLOG:[" << backlog << "]";
    sockFD_ = -1;
} 

SocketObj::SocketObj(int sockFD)
    : sockFD_(sockFD) {
}

SocketObj::~SocketObj() {
  Close();
}

void SocketObj::Dump() const {
  printf("sockFD_=%d", sockFD_);
}

int SocketObj::SetNonBlock(bool nonblock) {
  int flag = fcntl(sockFD_, F_GETFL, 0);
  if (nonblock) {
    // 设置成非阻塞模式
    return fcntl(sockFD_, F_SETFL, flag | O_NONBLOCK);
  } else {
    //设置为阻塞模式
    return fcntl(sockFD_, F_SETFL, flag & ~O_NONBLOCK);
  }
}


unsigned SocketObj::TranslateAddress() {
  if (strHost_ == "")
    return INADDR_ANY; 
  //return inet_addr(strHost_.c_str());
  //使用gethostbyname比使用inet_addr更好,从函数名上看比较清晰
  //而且还能根据域名来使用
  struct hostent *pstrHost_ = gethostbyname(strHost_.c_str());
  if (pstrHost_ == NULL) {
     return inet_addr(strHost_.c_str());
  }
  return *(int*)(pstrHost_->h_addr);
}

int SocketObj::Bind() {
  Close();
  sockFD_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFD_ == -1) {
    return -1;
  }
  struct sockaddr_in sAddr;
  memset(&sAddr, 0, sizeof(sAddr));
  sAddr.sin_addr.s_addr = TranslateAddress();
  sAddr.sin_family = AF_INET;
  sAddr.sin_port = htons(iPort_);
  return bind(sockFD_, (struct sockaddr*)&sAddr, sizeof(sAddr)); 
}

int SocketObj::Listen() {
  Close();
  if (Bind() != 0) {
    Close();
    return -1;
  }

  if (listen(sockFD_, backlog_) != 0) {
    Close();
    return -1;
  }
  return 0;
}

SocketObjPtr SocketObj::Accept() {
printf("zzzzzzzzzzzzzzzzzz\n");
  if (sockFD_ == -1) {
printf("xxxxxxxxxxxxxxxx\n");
    // 未经任何初始化的shared_ptr就指向一个NULL,这是一个magic,因为不能直接返回NULL
    SocketObjPtr tPtr;
    return tPtr;
  }
printf("cccccccccccccccccc\n");
  struct sockaddr_in sAddr;
  socklen_t length = sizeof(sAddr);
  int customFD = accept(sockFD_, (struct sockaddr*)&sAddr, &length);
  if (customFD == -1) {
printf("vvvvvvvvvvvvvvvv\n");
printf("-------------------customFD----------%d\n", customFD);
    SocketObjPtr tPtr;
    return tPtr;
  }
printf("bbbbbbbbbbbbbbb\n");
  SocketObjPtr tPtr(new SocketObj(customFD));
  return tPtr;
}

int SocketObj::Connect() {
  Close();
  sockFD_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFD_ == -1) {
    return -1;
  }
  struct sockaddr_in sAddr;
  memset(&sAddr, 0, sizeof(sAddr));
  sAddr.sin_family = AF_INET;
  sAddr.sin_port = htons(iPort_);
  sAddr.sin_addr.s_addr = TranslateAddress();
  if (connect(sockFD_, (struct sockaddr*)&sAddr, sizeof(sAddr))<0) {
    Close();
    return -1;
  }
  return 0; 
}

int SocketObj::Close() {
  if (sockFD_ != -1) {
    close(sockFD_);
    sockFD_ = -1;
  }
}

pair<string, int> SocketObj::GetPeer() {
  if (sockFD_ == -1) {
    return make_pair("", 0);
  }
  struct sockaddr_in sAddr;
  socklen_t length = sizeof(sAddr);
  if(getpeername(sockFD_, (struct sockaddr*)&sAddr, &length) != 0) {
    return make_pair("", 0);
  }
  string ipAddr = inet_ntoa(sAddr.sin_addr);
  int port = ntohs(sAddr.sin_port);
  return make_pair(ipAddr, port);
}

pair<string, int> SocketObj::GetSock() {
  if (sockFD_ == -1) {
    return make_pair("", 0);
  }
  struct sockaddr_in sAddr;
  socklen_t length = sizeof(sAddr);
  if(getsockname(sockFD_, (struct sockaddr*)&sAddr, &length) != 0) {
    return make_pair("", 0);
  }
  string ipAddr = inet_ntoa(sAddr.sin_addr);
  int port = ntohs(sAddr.sin_port);
  return make_pair(ipAddr, port);
}
