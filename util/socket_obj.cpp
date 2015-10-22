#include "SocketPool/socket_obj.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

SocketObj::SocketObj(string host, unsigned port)
    : strHost_(host),
      iPort_(port),
      backlog_(backlog) {
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

unsigned SocketObj::TranslateAddress() {
  if (strHost_ == "")
    return INADDR_ANY; 
  
  // 建立一个临时的socket
  int tmpSockFD = socket(AF_INET, SOCK_DGRAM, 0);
  // ifreq这个结构体的用处用来配置ip地址,激活接口,配置MTU等信息的
  struct ifreq sIfReq;
  strcpy(sIfReq.ifr_name, strHost_.c_str() + 1);
  strcpy(sIfReq.ifr_name, strHost_.c_str() + 1);
  if (0 != ioctl(tmpSockFD, SIOCGIFADDR, &sIfReq, sizeof(sIfReq))) {
    close(tmpSockFD); 
	return INADDR_NONE;
  }
  close(tmpSockFD); 
  struct sockaddr_in *pAddr = (struct sockaddr_in *)&(sIfReq.ifr_addr);
  return pAddr->sin_addr.s_addr;
}

int SocketObj::Bind() {
  Close();
  sockFD_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFD_ == -1) {
    return -1;
  }
  
  memset(&sAddr_, 0, sizeof(sAddr_));
  sAddr_.sin_addr.s_addr = TranslateAddress();
  sAddr_.sin_family = AF_INET;
  sAddr_.sin_port = htons(iPort_);
  return bind(sockFD_, (struct sockaddr*)&sAddr_, sizeof(sAddr_)); 
}

int SocketObj::Listen() {
  Close();
  Bind();
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
  if (sockFD_ == -1) {
    return NULL;
  }
  socklen_t length = sizeof(sAddr_);
  int customFD = accept(sockFD_, (struct sockaddr*)&sAddr_Len, &length) 
  if (customFD == -1) {
    return NULL;
  }
  boost::shared_ptr<SocketObj> tPtr(new SocketObj(customFD));
  return tPrt;
}

int SocketObj::Connect() {
  Close();
  sockFD_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFD_ == -1) {
    return -1;
  }

  memset(&sAddr_, 0, sizeof(sAddr_));
  sAddr_.sin_addr.s_addr = TranslateAddress(); 
  sAddr_.sin_family = AF_INET;
  sAddr_.sin_port = htons(iPort_);
  if (connect(sockFD_, (struct sockaddr*)&sAddr_, sizeof(sAddr_))!=0) {
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
