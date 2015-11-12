#include <iostream>
#include <event2/event.h>
#include <event2/bufferevent.h>

using namespace std;

const int LISTEN_PORT = 9999;
const int LISTEN_BACKLOG = 10;

void do_accept(evutil_socket_t listener, short event, void *arg);
void read_cb(struct bufferevent *bev, void *arg);
void write_cb(struct bufferevent *bev, void *arg);
void error_cb(struct bufferevent *bev, short event, void *arg);

int main(int argc, char** argv) {
  evutil_socket_t listener = socket(AF_INET, SOCK_STREAM, NULL);
  evutil_make_listen_socket_reuseable(listener);  
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  sin.sin_port = htons(LISTEN_PORT);  
  bind(listener, (struct sockaddr*)&sin, sizeof(sin));
  listen(listener, LISTEN_BACKLOG);
  evutil_make_socket_nonblocking(listener);


  //每个线程有且只有一个event_base,可以看作event的调度器
  struct event_base* base = event_base_new();
  //创建一个event,就是一个事件
  struct event* listen_event;
  //把event和一个函数绑定,listener是socket
  listen_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base);
  //将event托管给base,base作为调度器,当event这个事件发生的时候,base会在合适的时间去调用callback_func
  event_add(listen_event, NULL); 
  //启动base,让base调度event
  event_base_dispatch(base);
  
  cout << "The end" << endl;
  return 0;
}

void do_accept(evutil_socket_t listener, short event, void *arg) {
  struct event_base *base = (struct event_base*)arg;
  struct sockaddr_in sin;
  socklen_t slen = sizeof(sin);
  evutil_socket_t fd = accept(listener, (struct sockaddr*)&sin, &slen);
  struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, read_cb, write_cb, error_cb, arg);
  // 启用读写事件,其实是调用了event_add将相应读写事件加入事件监听队列poll
  bufferevent_enable(bev, EV_READ|EV_WRITE|EV_PERSIST); 
}
void read_cb(struct bufferevent *bev, void *arg) {
  char line[25];
  int n;
  evutil_socket_t fd = bufferevent_getfd(bev);
    n=bufferevent_read(bev, line, 25);
    line[n]='\0';
    cout << "read line:" << line << endl;
}

void write_cb(struct bufferevent *bev, void *arg) {
    char line[25] = {"hello,world"};
    int n;
    evutil_socket_t fd = bufferevent_getfd(bev);
    bufferevent_write(bev, line, n);
}
void error_cb(struct bufferevent *bev, short event, void *arg) {
  evutil_socket_t fd = bufferevent_getfd(bev);
  if (event & BEV_EVENT_TIMEOUT) {
    cout << "time out" << endl;
  }
  if (event & BEV_EVENT_EOF) {
    cout << "connection closed" << endl;
  }
  if (event & BEV_EVENT_ERROR) {
    cout << "some other error" << endl;
  }
}
