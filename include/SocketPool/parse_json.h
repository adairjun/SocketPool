#ifndef MQPOOL_INCLUDE_PARSE_JSON_H_
#define MQPOOL_INCLUDE_PARSE_JSON_H_

/* 以前一直都是把解析json文件的代码放在池的构造函数当中,这里把它独立出来
 *  使用boost的json解析库,也推荐使用rapidjson,
 * rapidjson是仿造rapidxml写出来的,不过没有用在boost库当中
 */

#include <string>
#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

using std::string;
using std::vector;
using std::map;
using std::make_pair;
using boost::property_tree::ptree;

typedef boost::shared_ptr<boost::property_tree::ptree> ptreePtr;

class ParseJsonObj {
 public:
  explicit ParseJsonObj();
  explicit ParseJsonObj(string configPath);
  virtual ~ParseJsonObj();
  ParseJsonObj(const ParseJsonObj&) = delete;
  ParseJsonObj& operator=(const ParseJsonObj&) = delete;
  void Dump() const;

  string GetConfigPath() const;

  ptreePtr GetPtree() const;

  /*
   * GetChildData("root.child.a") will get "a" 
   * { "root":
   *     { "child":
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *     }
   *  }
   */
  string GetChildData(const string& path);

  /*
   * GetChildData("root.child") will get "a" and "b"
   * { "root":
   *     { "child":
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *     }
   *  }
   */
  map<string, string> GetChildDataMap(const string& path);

  /*
   * GetChildDataArray("root.child") will get "a" and "b"
   * { "root":
   *     { "child":
   *        [
   *        { "a": "1",
   *          "b": "2" 
   *        },
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *        ]
   *     }
   *  }
   */
  vector<map<string, string> > GetChildDataArray(const string& path);

  //===========================================================
  /*
   * PutChildData("testput", "testput") will add "testput" in
   * { "root":
   *     { "child":
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *     }
   *  }
   * new:
   * { "root":
   *     { "child":
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *     },
   *   "testput": "testput"
   *  }
   */
  void PutChildData(const string& key, const string& value);

  /*
   * myMap.insert(make_pair("a", "1"))  myMap.insert(make_pair("b", "2"))
   * PutChildData("root.newchild", myMap) will add "newchild" in
   * { "root":
   *     { "child":
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *     }
   *  }
   * new:
   * { "root":
   *     { "child":
   *        { "a": "1",
   *          "b": "2" 
   *        },
   *        "newchild":
   *        { "a": "1",
   *          "b": "2"
   *        }
   *     }
   *  }
   */
  void PutChildDataMap(const string& key, const map<string, string>& key_value_map);

  /*
   * myMap.insert(make_pair("a", "1"))  myMap.insert(make_pair("b", "2"))
   * yourMap.insert(make_pair("a", "1"))  yourMap.insert(make_pair("b", "2"))
   * vector<map<string, string> > array;  array.push_back(myMap); array.push_back(yourMap);
   * PutChildData("root.nee", array) will add "nee" in
   * { "root":
   *     { "child":
   *        [
   *        { "a": "1",
   *          "b": "2" 
   *        },
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *        ]
   *     }
   * }
   * new:
   * { "root":
   *     { "child":
   *        [
   *        { "a": "1",
   *          "b": "2" 
   *        },
   *        { "a": "1",
   *          "b": "2" 
   *        }
   *        ],
   *        "nee": 
   *        [
   *        { "a": "1",
   *          "b": "2"
   *        },
   *        { "a": "1",
   *          "b": "2"
   *        }
   *        ]
   *     }
   * }
   */
  void PutChildDataArray(const string& key, const vector<map<string, string> >& array_list); 

  /*
   * save config
   */
  void SaveConfig();

 private:
  string configPath_; 
  //这里不用ptree对象而用指针的意义在于如果使用ptree对象的话，构造的时候就必须完全构造这个对象
  ptreePtr pt_;
  //ptree* pt_;
};

typedef boost::shared_ptr<ParseJsonObj> ParseJsonObjPtr;
#endif /* MQPOOL_INCLUDE_PARSE_JSON_H */
