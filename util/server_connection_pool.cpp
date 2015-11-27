#include "SocketPool/server_connection_pool.h"
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

ServerPool::ServerPool() {
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
  */
}
  
ServerPool::~ServerPool() {
  //析构函数做的工作是轮询map,让每个连接都close掉
  for (multimap<string, SocketObjPtr>::iterator sIt = server_map.begin(); sIt != server_map.end(); ++sIt) {
    sIt->second->Close();
  }
}

/**
 * Dump函数,专业debug30年!
 */
void ServerPool::Dump() const {
  printf("\n=====ServerPool Dump START ========== \n");
  printf("serverHost_=%s ", serverHost_.c_str());
  printf("serverPort_=%d ", serverPort_);
  printf("serverBacklog_=%d ", serverBacklog_);
  printf("serverPoolSize_=%d ", serverPoolSize_);
  printf("strErrorMessage_=%s\n ", strErrorMessage_.c_str());
  int count = 0;
  for (auto it = server_map.begin(); it!=server_map.end(); ++it) {
    printf("count==%d ", count);
    it->second->Dump();
    ++count;
  }
  printf("\n===ServerPool DUMP END ============\n");
}

bool ServerPool::Empty() const {
  int count = 0;
  for (auto it = server_map.begin(); it!=server_map.end(); ++it) {
    ++count;
  }
  if (count == 0) {
    return true;
  }
  return false;
}

/**
 * 从server_map当中选取一个连接
 * host和port是筛选的端口号
 */
SocketObjPtr ServerPool::GetConnection(string host, unsigned port) {
  // get connection operation
  unique_lock<mutex> lk(resource_mutex);
  char stringPort[10];
  snprintf(stringPort, sizeof(stringPort), "%d", port);
  string key = host + "###" + stringPort;
  multimap<string, SocketObjPtr>::iterator sIt = server_map.find(key);
  if (sIt != server_map.end()) {
    SocketObjPtr ret = sIt->second;
    server_map.erase(sIt);
    return ret;
  }
}
  
/**
 * 释放特定的连接,就是把SocketObjPtr放回到map当中
 * 其实严格地来说这个函数名字不应该叫做释放,而是插入,因为除了插入从池当中取出来的连接之外
 * 用户还能插入构造的连接,但是这样做没有意义
 */
bool ServerPool::ReleaseConnection(SocketObjPtr conn) {
  unique_lock<mutex> lk(resource_mutex);
  pair<string, int> sockPair = conn->GetSock();
  char stringPort[10];
  snprintf(stringPort, sizeof(stringPort), "%d", sockPair.second);
  string key = sockPair.first + "###" + stringPort;
  server_map.insert(make_pair(key, conn));
  return true;
}

/**
 * 构造函数创建poolsize个连接错误时候用来打印错误信息
 */
string ServerPool::ErrorMessage() const {
  return strErrorMessage_; 
} 
