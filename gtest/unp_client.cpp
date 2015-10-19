#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
const int PORT = 9999;
int main(int argc, char** argv) {
  int sockfd, n;
  char recvline[40];
  struct sockaddr_in servaddr;
  
  if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    cerr << "socket error" << endl;
  }

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);  
  if (inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr) <=0) {
    cerr << "inet_pton error for " << argv[1] << endl;
  }

  if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
    cerr << "connect error" << endl;
  }

  while ((n=read(sockfd, recvline, 40)) > 0) {
    recvline[n] = 0;
    if (fputs(recvline, stdout) == EOF) {
      cerr << "fputs error" << endl;
    }
  }

  close(sockfd);
  return 0;
}
