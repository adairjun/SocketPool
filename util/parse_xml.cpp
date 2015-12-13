#include "SocketPool/parse_xml.h"
#include <boost/property_tree/xml_parser.hpp>


ParseXmlObj::ParseXmlObj() 
    : pt_(new ptree) {
    configPath_ = "../config/messageQueue.xml";
    //pt_ = new ptree;
    boost::property_tree::read_xml(configPath_, *pt_);
}

ParseXmlObj::ParseXmlObj(string configPath)
    : configPath_(configPath), pt_(new ptree) {
    //pt_ = new ptree;
	boost::property_tree::read_xml(configPath_, *pt_);
}

ParseXmlObj::~ParseXmlObj() {
  //delete pt_;
}

void ParseXmlObj::Dump() const {
  printf("\n=====ParseXmlObj Dump START ========== \n");
  printf("configPath__=%s ", configPath_.c_str());
  //printf("pt_=%p ", pt_);
  printf("\n===ParseXMlObj DUMP END ============\n");
}

string ParseXmlObj::GetConfigPath() const {
	return configPath_;
}

ptreePtr ParseXmlObj::GetPtree() const {
	return pt_;
}

string ParseXmlObj::GetChildData(const string& path) {
  return pt_->get<string>(path);
}

map<string, string> ParseXmlObj::GetChildDataMap(const string& path) {
  map<string, string> key_value_map;
 
  auto child = pt_->get_child(path);
  for (auto pos = child.begin(); pos!= child.end(); ++pos) {
    key_value_map.insert(make_pair(pos->first, pos->second.data()));
  }
  return std::move(key_value_map);
}

vector<map<string, string> > ParseXmlObj::GetChildDataArray(const string& path) {
  vector<map<string, string> > result_array;
  map<string, string> key_value_map;
 
  auto child = pt_->get_child(path);
  for (auto pos = child.begin(); pos!= child.end(); ++pos) {
    auto nextchild = pos->second.get_child("");
    for (auto nextpos = nextchild.begin(); nextpos!= nextchild.end(); ++nextpos) {
      key_value_map.insert(make_pair(nextpos->first, nextpos->second.data()));
    }
    result_array.push_back(key_value_map);
    key_value_map.clear();
  }
  return std::move(result_array);
}

string ParseXmlObj::GetAttr(string path, const string& attr) {
  if(pt_ == NULL || attr == "")
    return "";
  if(path != ""){
    path += "."; //add the domain descriptor
  }
  path += "<xmlattr>.";
  path += attr;
  return pt_->get<string>(path, "");
}

vector<string> ParseXmlObj::GetAttrArray(string path, const string& attr) {
  vector<string> result;
  if (pt_ != NULL && attr != "") {
    auto child = pt_->get_child(path);
    for (auto pos = child.begin(); pos!= child.end(); ++pos) {
      auto nextchild = pos->second.get_child("");
      for (auto nextpos = nextchild.begin(); nextpos!= nextchild.end(); ++nextpos) {
        result.push_back(nextpos->second.get<string>(attr));
      }
    }
  }
  return std::move(result);
}

string ParseXmlObj::GetAttrByAttr(string path, const string& know_attr, const string& know_value, const string& attr) {
  string result = "";
  auto child = pt_->get_child(path);
  for (auto pos = child.begin(); pos!= child.end(); ++pos) {
    auto nextchild = pos->second.get_child("");
    for (auto nextpos = nextchild.begin(); nextpos!= nextchild.end(); ++nextpos) {
      if (nextpos->second.get<string>(know_attr) == know_value) {
        result = nextpos->second.get<string>(attr); 
        break;
      }
    }
  }
  return result;
}


void ParseXmlObj::PutChildData(const string& key, const string& value) {
  if(pt_ != NULL) {
    pt_->put(key, value);
  }
}

void ParseXmlObj::PutChildDataMap(const string& key, const map<string, string>& key_value_map) {
  if(pt_ != NULL) {
    ptree child;
    for (auto myPair : key_value_map) {
      child.put(myPair.first, myPair.second);
    }
    pt_->add_child(key, child);
  }
}

void ParseXmlObj::PutAttr(string path, const string& attribute, const string& attrvalue) {
  if (pt_ != NULL) {
	if(path != "") {
	  path += "."; //add the domain descriptor
	}
	path += "<xmlattr>.";
	path += attribute;
	pt_->put<string>(path, attrvalue);
  }
}

void ParseXmlObj::AddLineByAttr(string path, const map<string, string>& key_value_map) {
  if (pt_ != NULL) {
    ptree temp;
    for (auto myPair : key_value_map) {
      temp.put<string>("<xmlattr>." + myPair.first, myPair.second.data());  
    } 
    pt_->add_child(path, temp);
  }
}

void ParseXmlObj::SaveConfig() {
  if(pt_ != NULL) {
    boost::property_tree::write_xml(configPath_, *pt_);
  }
}
