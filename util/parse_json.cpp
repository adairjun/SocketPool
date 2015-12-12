#include "SocketPool/parse_json.h"
#include <boost/property_tree/json_parser.hpp>


ParseJsonObj::ParseJsonObj() {
    configPath_ = "../config/messageQueue.json";
    pt_ = new ptree;
    boost::property_tree::read_json(configPath_, *pt_);
}

ParseJsonObj::ParseJsonObj(string configPath)
    : configPath_(configPath){
	pt_ = new ptree;
	boost::property_tree::read_json(configPath_, *pt_);
}

ParseJsonObj::~ParseJsonObj() {
  delete pt_;
}

void ParseJsonObj::Dump() const {
  printf("\n=====ParseJsonObj Dump START ========== \n");
  printf("configPath__=%s ", configPath_.c_str());
  printf("pt_=%p ", pt_);
  printf("\n===ParseJsonObj DUMP END ============\n");
}

string ParseJsonObj::GetConfigPath() const {
  return configPath_;
}

ptree* ParseJsonObj::GetPtree() const {
  return pt_;
}

string ParseJsonObj::GetChildData(const string& path) {
  return pt_->get<string>(path);
}

map<string, string> ParseJsonObj::GetChildDataMap(const string& path) {
  map<string, string> key_value_map;
  
  auto child = pt_->get_child(path);
  for (auto pos = child.begin(); pos!= child.end(); ++pos) {
    key_value_map.insert(make_pair(pos->first, pos->second.data()));
  }
  return std::move(key_value_map);
}

vector<map<string, string> > ParseJsonObj::GetChildDataArray(const string& path) {
  vector<map<string, string> > result_array;
  map<string, string> key_value_map;

  auto child = pt_->get_child(path);
    for (ptree::value_type &v : child.get_child("")) {
      auto nextchild = v.second.get_child("");
      for (auto pos = nextchild.begin(); pos!= nextchild.end(); ++pos) {
        key_value_map.insert(make_pair(pos->first, pos->second.data()));        
      }
      result_array.push_back(key_value_map);
      key_value_map.clear();
    }
  return std::move(result_array);
}

void ParseJsonObj::PutChildData(const string& key, const string& value) {
  if(pt_ != NULL) {
    pt_->put(key, value);
  }
}

void ParseJsonObj::PutChildDataMap(const string& key, const map<string, string>& key_value_map) {
  if(pt_ != NULL) {
    ptree child;
    for (auto myPair : key_value_map) {
      child.put(myPair.first, myPair.second);
    }
    pt_->add_child(key, child);
  }
}

void ParseJsonObj::PutChildDataArray(const string& key, const vector<map<string, string> >& array_list) {
  if(pt_ != NULL) {
    ptree first;
    for (auto key_value_map : array_list) {
      ptree second;
      for (auto myPair : key_value_map) {
        second.put(myPair.first, myPair.second);
      } 
      first.push_back(make_pair("", second)); 
    }
    pt_->add_child(key, first);
  }
}

void ParseJsonObj::SaveConfig() {
  if(pt_ != NULL) {
    boost::property_tree::write_json(configPath_, *pt_);
  }
}
