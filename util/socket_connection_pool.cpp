#include "SocketPool/socket_connection_pool.h"
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;

SocketPool::SocketPool() {
  // 从配置文件socket.xml当中读入mysql的ip, 用户, 密码, 数据库名称,	
  boost::property_tree::ptree pt;	
  const char* xml_path = "../config/socket.xml";	
  boost::property_tree::read_xml(xml_path, pt);
  
  //这段注释的代码是读取json配置文件的
  //	const char* json_path = "../config/socket.json";
  //	boost::property_tree::read_json(json_path, pt);
  
  // 先做好server_list
  BOOST_AUTO(child, pt.get_child("Config.ServerConnection"));
  for (BOOST_AUTO(pos, child.begin()); pos!= child.end(); ++pos) {
  	if (pos->first == "IP") serverHost_ = pos->second.data();
  	if (pos->first == "Port") serverPort_ = boost::lexical_cast<int>(pos->second.data());
  	if (pos->first == "Backlog") serverBacklog_ = boost::lexical_cast<int>(pos->second.data());
  	if (pos->first == "max_connections") serverPoolSize_ = boost::lexical_cast<int>(pos->second.data());
  }
cout << "backlog================" << serverBacklog_ << endl;
  // 构造函数的作用就是根据poolSize的大小来构造多个映射
  // 每个映射的连接都是同样的host,port,backlog
  
  for (int i=0; i<serverPoolSize_; ++i) {
  	SocketObjPtr conn(new SocketObj(serverHost_, serverPort_, serverBacklog_));
    //Listen()当中已经封装了bind
  	if (conn->Listen()) {
  	  server_list.push_back(conn);
  	} else {
  	  strErrorMessage_ = conn->ErrorMessage();
  	}
  }

  //client_list
  BOOST_AUTO(child, pt.get_child("Config.ClientConnection"));
  for (BOOST_AUTO(pos, child.begin()); pos!= child.end(); ++pos) {
  	if (pos->first == "IP") clientConnectHost_ = pos->second.data();
  	if (pos->first == "Port") clientConnectPort_ = boost::lexical_cast<int>(pos->second.data());
  	if (pos->first == "Backlog") clientConnectBacklog_ = boost::lexical_cast<int>(pos->second.data());
  	if (pos->first == "max_connections") clientPoolSize_ = boost::lexical_cast<int>(pos->second.data());
  }
  // 构造函数的作用就是根据poolSize的大小来构造多个映射
  // 每个映射的连接都是同样的host,port,backlog
  
  for (int i=0; i<clientPoolSize_; ++i) {
  	SocketObjPtr conn(new SocketObj(clientConnectHost_, clientConnectPort_, clientConnectBacklog_));
    //只有server启动了,client的connect才会成功
    //所以要先写server_list
  	if (conn->Connect()) {
  	  client_list.push_back(conn);
  	} else {
  	  strErrorMessage_ = conn->ErrorMessage();
  	}
  }
}
  
SocketPool::~SocketPool() {
}

/**
 * 从list当中选取一个连接,如果传入true,那么就从server_list当中选一个连接
 */
SocketObjPtr SocketPool::GetConnection(bool server) {
  // get connection operation
  unique_lock<mutex> lk(resource_mutex);
  if (server) {
    if (!server_list.empty()) {
      SocketObjPtr ret = server_list.front();
      server_list.pop_front();
    }
  } else {
    if (!client_list.empty()) {
      SocketObjPtr ret = client_list.front();
      client_list.pop_front();
    }
  }
}
  
/**
 * 释放特定的连接,就是把SocketObjPtr放回到list当中
 * 第一个参数指定的是这个SocketObjPtr是server或者是client
 */
int SocketPool::ReleaseConnection(bool server, SocketObjPtr conn) {
  unique_lock<mutex> lk(resource_mutex);
  if (server) {
    server_list.push_back(conn);
  }
  return 1;
}

/**
 * 构造函数创建poolsize个连接错误时候用来打印错误信息
 */
string SocketPool::ErrorMessage() const {
  return strErrorMessage_; 
} 
