#include "SocketPool/socket_connection_pool.h"
#include <iostream>
#include <stdlib.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;

SocketPool::SocketPool() {
printf("[FILE:%s]:line:%d\n", __FILE__, __LINE__);
  //这里我使用boost来解析xml和json配置文件,也可以使用rapidxml或者rapidjson
  // 从配置文件socket.xml当中读入mysql的ip, 用户, 密码, 数据库名称,	
  boost::property_tree::ptree pt;	
  const char* xml_path = "../config/socket.xml";	
  boost::property_tree::read_xml(xml_path, pt);
  
  // 先做好server_map
  BOOST_AUTO(child, pt.get_child("Config.Server"));
  for (BOOST_AUTO(pos, child.begin()); pos!= child.end(); ++pos) {
    BOOST_AUTO(nextchild, pos->second.get_child(""));
    for (BOOST_AUTO(nextpos, nextchild.begin()); nextpos!= nextchild.end(); ++nextpos) {
  	  if (nextpos->first == "IP") serverHost_ = nextpos->second.data();
  	  if (nextpos->first == "Port") serverPort_ = boost::lexical_cast<unsigned>(nextpos->second.data());
  	  if (nextpos->first == "Backlog") serverBacklog_ = boost::lexical_cast<int>(nextpos->second.data());
    }
    // 构造函数的作用就是根据poolSize的大小来构造多个映射
    // 每个映射的连接都是同样的host,port,backlog

    SocketObjPtr conn(new SocketObj(serverHost_, serverPort_, serverBacklog_));
    //Listen()当中已经封装了bind
    if (conn->Listen()) {
      //在insert之前需要先把ip和端口号整合成key,先对端口号做处理,把int转为string
      char stringPort[10];
      snprintf(stringPort, sizeof(stringPort), "%d", serverPort_);
      string key = serverHost_ + "###" + stringPort;
      server_map.insert(make_pair(key, conn));
    } else {
      strErrorMessage_ = conn->ErrorMessage();
    }
  }

  //client_map
  BOOST_AUTO(childClient, pt.get_child("Config.Client"));
  for (BOOST_AUTO(pos, childClient.begin()); pos!= childClient.end(); ++pos) {
    BOOST_AUTO(nextchild, pos->second.get_child(""));
    for (BOOST_AUTO(nextpos, nextchild.begin()); nextpos!= nextchild.end(); ++nextpos) {
  	  if (nextpos->first == "IP") clientConnectHost_ = nextpos->second.data();
  	  if (nextpos->first == "Port") clientConnectPort_ = boost::lexical_cast<int>(nextpos->second.data());
  	  if (nextpos->first == "Backlog") clientConnectBacklog_ = boost::lexical_cast<int>(nextpos->second.data());
  	  if (nextpos->first == "max_connections") clientPoolSize_ = boost::lexical_cast<int>(nextpos->second.data());
    }
    // 构造函数的作用就是根据poolSize的大小来构造多个映射
    // 每个映射的连接都是同样的host,port,backlog
    
    for (int i=0; i<clientPoolSize_; ++i) {
      SocketObjPtr conn(new SocketObj(clientConnectHost_, clientConnectPort_, clientConnectBacklog_));
      //只有server启动了,client的connect才会成功
      //所以要先写server_map
      if (conn->Connect()) {
        char stringPort[10];
        snprintf(stringPort, sizeof(stringPort), "%d", clientConnectPort_);
        string key = clientConnectHost_ + "###" + stringPort;
        client_map.insert(make_pair(key, conn));
      } else {
    	strErrorMessage_ = conn->ErrorMessage();
      }
    }
  }

  /**
   * 由于json当中使用数组来保存Connection,这里不能通用代码
   * 这段注释的代码是读取json配置文件的
  const char* json_path = "../config/socket.json";
  boost::property_tree::read_json(json_path, pt);
  BOOST_AUTO(child, pt.get_child("Config.Server"));
  for (BOOST_AUTO(pos, child.begin()); pos!= child.end(); ++pos) {
   //这里应该遍历数组
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pos->second.get_child("")) {
      BOOST_AUTO(nextchild, v.second.get_child(""));
      for (BOOST_AUTO(nextpos, nextchild.begin()); nextpos!= nextchild.end(); ++nextpos) {
  	    if (nextpos->first == "IP") serverHost_ = nextpos->second.data();
  	    if (nextpos->first == "Port") serverPort_ = boost::lexical_cast<unsigned>(nextpos->second.data());
  	    if (nextpos->first == "Backlog") serverBacklog_ = boost::lexical_cast<int>(nextpos->second.data());
      } 
      SocketObjPtr conn(new SocketObj(serverHost_, serverPort_, serverBacklog_));
      //Listen()当中已经封装了bind
      if (conn->Listen()) {
        char stringPort[10];
        snprintf(stringPort, sizeof(stringPort), "%d", serverPort_);
        string key = serverHost_ + "###" + stringPort;
        server_map.insert(make_pair(key, conn));
      } else {
        strErrorMessage_ = conn->ErrorMessage();
      }
    }
  }

  //client_map
  BOOST_AUTO(childClient, pt.get_child("Config.Client"));
  for (BOOST_AUTO(pos, childClient.begin()); pos!= childClient.end(); ++pos) {
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pos->second.get_child("")) {
      BOOST_AUTO(nextchild, v.second.get_child(""));
      for (BOOST_AUTO(nextpos, nextchild.begin()); nextpos!= nextchild.end(); ++nextpos) {
  	    if (nextpos->first == "IP") clientConnectHost_ = nextpos->second.data();
  	    if (nextpos->first == "Port") clientConnectPort_ = boost::lexical_cast<int>(nextpos->second.data());
  	    if (nextpos->first == "Backlog") clientConnectBacklog_ = boost::lexical_cast<int>(nextpos->second.data());
  	    if (nextpos->first == "max_connections") clientPoolSize_ = boost::lexical_cast<int>(nextpos->second.data());
      }
      for (int i=0; i<clientPoolSize_; ++i) {
        SocketObjPtr conn(new SocketObj(clientConnectHost_, clientConnectPort_, clientConnectBacklog_));
        //只有server启动了,client的connect才会成功
        //所以要先写server_map
        if (conn->Connect()) {
          char stringPort[10];
          snprintf(stringPort, sizeof(stringPort), "%d", clientConnectPort_);
          string key = clientConnectHost_ + "###" + stringPort;
          client_map.insert(make_pair(key, conn));
        } else {
      	  strErrorMessage_ = conn->ErrorMessage();
        }
      }
    }
  }
  */
printf("[FILE:%s]:line:%d\n", __FILE__, __LINE__);
}
  
SocketPool::~SocketPool() {
printf("[FILE:%s]:line:%d\n", __FILE__, __LINE__);
  //析构函数做的工作是轮询map,让每个连接都close掉
  //先close掉server
  for (multimap<string, SocketObjPtr>::iterator sIt = server_map.begin(); sIt != server_map.end(); ++sIt) {
    sIt->second->Close();
  }
  //再close掉client
  for (multimap<string, SocketObjPtr>::iterator sIt = client_map.begin(); sIt != client_map.end(); ++sIt) {
    sIt->second->Close();
  }
}

/**
 * 从list当中选取一个连接,如果传入true,那么就从server_list当中选一个连接
 * host和port是筛选的端口号
 */
SocketObjPtr SocketPool::GetConnection(bool server, string host, unsigned port) {
printf("[FILE:%s]:line:%d\n", __FILE__, __LINE__);
  // get connection operation
  unique_lock<mutex> lk(resource_mutex);
  char stringPort[10];
  snprintf(stringPort, sizeof(stringPort), "%d", port);
  string key = host + "###" + stringPort;
  if (server) {
	multimap<string, SocketObjPtr>::iterator sIt = server_map.find(key);
    if (sIt != server_map.end()) {
      SocketObjPtr ret = sIt->second;
      server_map.erase(sIt);
      return ret;
    }
  } else {
	multimap<string, SocketObjPtr>::iterator sIt = client_map.find(key);
    if (sIt != client_map.end()) {
      SocketObjPtr ret = sIt->second;
      client_map.erase(sIt);
      return ret;
	}
  }
}
  
/**
 * 释放特定的连接,就是把SocketObjPtr放回到list当中
 * 第一个参数指定的是这个SocketObjPtr是server或者是client
 * 其实严格地来说这个函数名字不应该叫做释放,而是插入,因为除了插入从池当中取出来的连接之外
 * 用户还能插入构造的连接,但是这样做没有意义
 */
int SocketPool::ReleaseConnection(bool server, SocketObjPtr conn) {
printf("[FILE:%s]:line:%d\n", __FILE__, __LINE__);
  unique_lock<mutex> lk(resource_mutex);
  if (server) {
    pair<string, int> sockPair = conn->GetSock();
    char stringPort[10];
    snprintf(stringPort, sizeof(stringPort), "%d", sockPair.second);
    string key = sockPair.first + "###" + stringPort;
    server_map.insert(make_pair(key, conn));
  } else {
    pair<string, int> peerPair= conn->GetPeer();
    char stringPort[10];
    snprintf(stringPort, sizeof(stringPort), "%d", peerPair.second);
    string key = peerPair.first + "###" + stringPort;
    client_map.insert(make_pair(key, conn));
  }
  return 1;
}

/**
 * 构造函数创建poolsize个连接错误时候用来打印错误信息
 */
string SocketPool::ErrorMessage() const {
  return strErrorMessage_; 
} 
