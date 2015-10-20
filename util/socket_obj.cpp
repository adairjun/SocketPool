#include "SocketPool/socket_obj.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

SocketObj::SocketObj(string host, unsigned port)
    : m_strHost(host),
      m_iPort(port) {
    m_sockFD = -1;
} 

SocketObj::SocketObj(int sockFD)
    : m_sockFD(sockFD) {
}

SocketObj::~SocketObj() {
  Close();
}

void SocketObj::Dump() const {
  printf("m_sockFD=%d", m_sockFD);
}

unsigned SocketObj::TranslateAddress() {
  if (m_strHost == "")
    return INADDR_ANY; 
  
  // 建立一个临时的socket
  int tmpSockFD = socket(AF_INET, SOCK_DGRAM, 0);
  // ifreq这个结构体的用处用来配置ip地址,激活接口,配置MTU等信息的
  struct ifreq sIfReq;
  strcpy(sIfReq.ifr_name, m_strHost.c_str() + 1);
  strcpy(sIfReq.ifr_name, m_strHost.c_str() + 1);
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
  m_sockFD = socket(AF_INET, SOCK_STREAM, 0);
  if (m_sockFD == -1) {
    return -1;
  }
  
  memset(&m_sAddr, 0, sizeof(m_sAddr));
  m_sAddr.sin_addr.s_addr = TranslateAddress();
  m_sAddr.sin_family = AF_INET;
  m_sAddr.sin_port = htons(m_iPort);
  return bind(m_sockFD, (struct sockaddr*)&m_sAddr, sizeof(m_sAddr)); 
}

int SocketObj::Listen() {
  Close();
  Bind();
  if (Bind() != 0) {
    Close();
    return -1;
  }

  if (listen(m_sockFD, backlog) != 0) {
    Close();
    return -1;
  }
  return 0;
}

SocketObjPtr SocketObj::Accept() {
  if (m_sockFD == -1) {
    return NULL;
  }
  socklen_t length = sizeof(m_sAddr);
  int customFD = accept(m_sockFD, (struct sockaddr*)&m_sAddrLen, &length) 
  if (customFD == -1) {
    return NULL;
  }
  boost::shared_ptr<SocketObj> tPtr(new SocketObj(customFD));
  return tPrt;
}

int SocketObj::Connect() {
  Close();
  m_sockFD = socket(AF_INET, SOCK_STREAM, 0);
  if (m_sockFD == -1) {
    return -1;
  }

  memset(&m_sAddr, 0, sizeof(m_sAddr));
  m_sAddr.sin_addr.s_addr = TranslateAddress(); 
  m_sAddr.sin_family = AF_INET;
  m_sAddr.sin_port = htons(m_iPort);
  if (connect(m_sockFD, (struct sockaddr*)&m_sAddr, sizeof(m_sAddr))!=0) {
    Close();
    return -1;
  }
  return 0; 
}

int SocketObj::Close() {
  if (m_sockFD != -1) {
    close(m_sockFD);
    m_sockFD = -1;
  }
}
