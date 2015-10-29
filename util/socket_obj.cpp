#include "SocketPool/socket_obj.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

SocketObj::SocketObj(string host, unsigned port, int backlog)
    : strHost_(host),
      iPort_(port),
      backlog_(backlog) {
    sockFD_ = -1;
    psAddr_->sin_family = AF_INET;
    psAddr_->sin_port = htons(iPort_);
    psAddr_->sin_addr.s_addr = INADDR_ANY;
} 

SocketObj::SocketObj(int sockFD)
    : sockFD_(sockFD) {
    psAddr_->sin_family = AF_INET;
    psAddr_->sin_port = htons(iPort_);
    psAddr_->sin_addr.s_addr = INADDR_ANY;
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
  strcpy(sIfReq.ifr_name, strHost_.c_str() + 1); strcpy(sIfReq.ifr_name, strHost_.c_str() + 1);
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
  
  memset(psAddr_, 0, sizeof(*psAddr_));
  psAddr_->sin_addr.s_addr = TranslateAddress();
  psAddr_->sin_family = AF_INET;
  psAddr_->sin_port = htons(iPort_);
  return bind(sockFD_, (struct sockaddr*)psAddr_, sizeof(*psAddr_)); 
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
    // 未经任何初始化的shared_ptr就指向一个NULL,这是一个magic,因为不能直接返回NULL
    SocketObjPtr tPtr;
    return tPtr;
  }
  socklen_t length = sizeof(*psAddr_);
  int customFD = accept(sockFD_, (struct sockaddr*)psAddr_, &length);
  if (customFD == -1) {
    SocketObjPtr tPtr;
    return tPtr;
  }
  SocketObjPtr tPtr(new SocketObj(customFD));
  return tPtr;
}

int SocketObj::Connect() {
  Close();
  sockFD_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFD_ == -1) {
    return -1;
  }

  memset(psAddr_, 0, sizeof(*psAddr_));
  psAddr_->sin_addr.s_addr = TranslateAddress(); 
  psAddr_->sin_family = AF_INET;
  psAddr_->sin_port = htons(iPort_);
  if (connect(sockFD_, (struct sockaddr*)psAddr_, sizeof(*psAddr_))!=0) {
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
