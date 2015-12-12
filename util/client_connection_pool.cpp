#include "SocketPool/client_connection_pool.h"
#include "SocketPool/parse_xml.h"
#include "SocketPool/parse_json.h"
#include <iostream>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>

using std::cout;
using std::endl;

ClientPool::ClientPool() {
  ParseXmlObj myXml("../config/socket.xml");
  vector<map<string, string> > result_array = myXml.GetChildDataArray("Config.Client"); 
  //  这段注释的代码是读取json配置文件的
  //ParseJsonObj myJson("../config/socket.json");
  //vector<map<string, string> > result_array = myJson.GetChildDataArray("Config.Client.Connection"); 
  for (auto key_value_map : result_array) {
    clientConnectHost_ = key_value_map["IP"];
    clientConnectPort_ = boost::lexical_cast<int>(key_value_map["Port"]);
    clientPoolSize_ = boost::lexical_cast<int>(key_value_map["max_connections"]);

    for (int i=0; i<clientPoolSize_; ++i) {
      SocketObjPtr conn(new SocketObj(clientConnectHost_, clientConnectPort_));
      //只有server启动了,client的connect才会成功
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

  //client_map
  BOOST_AUTO(childClient, pt.get_child("Config.Client"));
  for (BOOST_AUTO(pos, childClient.begin()); pos!= childClient.end(); ++pos) {
    BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pos->second.get_child("")) {
      BOOST_AUTO(nextchild, v.second.get_child(""));
      for (BOOST_AUTO(nextpos, nextchild.begin()); nextpos!= nextchild.end(); ++nextpos) {
  	    if (nextpos->first == "IP") clientConnectHost_ = nextpos->second.data();
  	    if (nextpos->first == "Port") clientConnectPort_ = boost::lexical_cast<int>(nextpos->second.data());
  	    if (nextpos->first == "max_connections") clientPoolSize_ = boost::lexical_cast<int>(nextpos->second.data());
      }
      for (int i=0; i<clientPoolSize_; ++i) {
        SocketObjPtr conn(new SocketObj(clientConnectHost_, clientConnectPort_));
        //只有server启动了,client的connect才会成功
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
}
  
ClientPool::~ClientPool() {
  //析构函数做的工作是轮询map,让每个连接都close掉
  //再close掉client
  for (multimap<string, SocketObjPtr>::iterator sIt = client_map.begin(); sIt != client_map.end(); ++sIt) {
    sIt->second->Close();
  }
}

/**
 * Dump函数,专业debug30年!
 */
void ClientPool::Dump() const {
  printf("\n=====ClientPool Dump START ========== \n");
  printf("clientConnectHost_=%s ", clientConnectHost_.c_str());
  printf("clientConnectPort_=%d ", clientConnectPort_);
  printf("clientPoolSize_=%d ", clientPoolSize_);
  printf("strErrorMessage_=%s\n ", strErrorMessage_.c_str());
  int count = 0;
  for (auto it = client_map.begin(); it!=client_map.end(); ++it) {
    printf("count==%d ", count);
    it->second->Dump();
    ++count;
  }
  printf("\n===ClientPool DUMP END ============\n");
}

bool ClientPool::Empty() const {
  return client_map.empty();
}

/**
 * 从map当中选取一个连接
 * host和port是筛选的端口号
 */
SocketObjPtr ClientPool::GetConnection(string host, unsigned port) {
  // get connection operation
  unique_lock<mutex> lk(resource_mutex);
  char stringPort[10];
  snprintf(stringPort, sizeof(stringPort), "%d", port);
  string key = host + "###" + stringPort;
  multimap<string, SocketObjPtr>::iterator sIt = client_map.find(key);
  if (sIt != client_map.end()) {
    SocketObjPtr ret = sIt->second;
    client_map.erase(sIt);
    return ret;
  }
}
  
/**
 * 释放特定的连接,就是把SocketObjPtr放回到list当中
 * 其实严格地来说这个函数名字不应该叫做释放,而是插入,因为除了插入从池当中取出来的连接之外
 * 用户还能插入构造的连接,但是这样做没有意义
 */
bool ClientPool::ReleaseConnection(SocketObjPtr conn) {
  unique_lock<mutex> lk(resource_mutex);
  pair<string, int> peerPair= conn->GetPeer();
  char stringPort[10];
  snprintf(stringPort, sizeof(stringPort), "%d", peerPair.second);
  string key = peerPair.first + "###" + stringPort;
  client_map.insert(make_pair(key, conn));
  return true;
}

/**
 * 构造函数创建poolsize个连接错误时候用来打印错误信息
 */
string ClientPool::ErrorMessage() const {
  return strErrorMessage_; 
} 
