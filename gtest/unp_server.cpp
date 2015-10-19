#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
const int PORT = 9999;
int main(int argc, char** argv) {
  int sockfd;
  struct sockaddr_in servaddr;
  struct sockaddr_in cliaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(PORT);
  servaddr.sin_addr.s_addr = INADDR_ANY; 
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
  listen(sockfd, 10);
  while (true) {
    socklen_t length = sizeof(cliaddr);
    int clifd = accept(sockfd, (struct sockaddr*) &servaddr, &length);
    send(clifd, "helloworld", 10, 0);
    close(clifd);
  }

  close(sockfd);
  return 0;
}
  
