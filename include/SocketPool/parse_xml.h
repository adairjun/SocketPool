#ifndef MQPOOL_INCLUDE_PARSE_XML_H_
#define MQPOOL_INCLUDE_PARSE_XML_H_

/* 以前一直都是把解析xml文件的代码放在池的构造函数当中,这里把它独立出来
 *  使用boost的xml解析库,也推荐使用rapidxml,而且boost的xml解析库用的就是rapidxml
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

class ParseXmlObj {
 public:
  explicit ParseXmlObj();
  explicit ParseXmlObj(string configPath);
  virtual ~ParseXmlObj();
  ParseXmlObj(const ParseXmlObj&) = delete;
  ParseXmlObj& operator=(const ParseXmlObj&) = delete;
  void Dump() const;

  string GetConfigPath() const;

  ptreePtr GetPtree() const;

  /*
   * GetChildData("root.child.a") will get <a> 
   * <root>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   * </root>
   */
  string GetChildData(const string& path);

  /*
   * GetChildData("root.child") will get <a> and <b>
   * <root>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   * </root>
   */
  map<string, string> GetChildDataMap(const string& path);

  /*
   * 请注意这里和parse_json当中的GetChildData的区别，严格的来说xml并没有数组的概念
   * 这里只是为了方便拿数据所以实现了这个接口
   * GetChildData("root") will get <a> and <b>
   * <root>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   * </root>
   */
  vector<map<string, string> > GetChildDataArray(const string& path);

  /*
   * get attribute by path
   * GetAttr("errors.error", "id") get the first attribute
   *  <errors>
   *  <error id="DB_ERROR_EXECUTE" value="1" prompt="操作数据库失败" msg="操作数据库失败" test="ttttt"/>
   *  <error id="DB_ERROR_COMMAND" value="2" prompt="创建数据库操作指令失败" msg="创建数据库操作指令失败"/>
   *  </errors>
   * the result is "1"
   */
  string GetAttr(string path, const string& attr);

  /*
   * get attribute by path
   * GetAttrArray("errors", "id") get the first attribute
   *  <errors>
   *  <error id="DB_ERROR_EXECUTE" value="1" prompt="操作数据库失败" msg="操作数据库失败" test="ttttt"/>
   *  <error id="DB_ERROR_COMMAND" value="2" prompt="创建数据库操作指令失败" msg="创建数据库操作指令失败"/>
   *  </errors>
   * the result is a vector
   */
  vector<string> GetAttrArray(string path, const string& attr);

  /*
   * get attribute by path and attribute
   * GetAttr("errors", "id", "DB_ERROR_EXECUTE", "value") get the "value" attribute where "id" is "DB_ERROR_EXECUTE"
   * 注意这个接口的path传的是errors 而不是errors.error
   *  <errors>
   *  <error id="DB_ERROR_EXECUTE" value="1" prompt="操作数据库失败" msg="操作数据库失败" test="ttttt"/>
   *  <error id="DB_ERROR_COMMAND" value="2" prompt="创建数据库操作指令失败" msg="创建数据库操作指令失败"/>
   *  </errors>
   * result is "1"
   */
  string GetAttrByAttr(string path, const string& know_attr, const string& know_value, const string& attr);

  //===========================================================
  /*
   * PutChildData("testput", "testput") will add "testput" in
   * <root>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   * </root>
   * new:
   * <root>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   * </root>
   * <testput>"testput"</testput>
   */
  void PutChildData(const string& key, const string& value);

  /*
   * myMap.insert(make_pair("a", "1"))  myMap.insert(make_pair("b", "2"))
   * PutChildData("root.newchild", myMap) will add "newchild" in
   * <root>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   * </root>
   * new:
   * <root>
   *   <child>
   *     <a></a>
   *     <b></b>
   *   </child>
   *   <newchild>
   *     <a></a>
   *     <b></b>
   *   </newchild>
   * </root> 
   */
  void PutChildDataMap(const string& key, const map<string, string>& key_value_map);

  /*
   * 上面说过了xml并没有数组的概念,所以这里没有 PutChildDataArray()函数
   */

  /*
   * PutAttr("errors.error", "id", "DB_ERROR_EXECUTE") 
   * <error id="DB_ERROR_EXECUTE"/>
   */
  void PutAttr(string path, const string& attribute, const string& attrvalue);

  /*
   * 这个通过attribute找到对应的标签，再在标签中加入一个attribute的功能很难实现,所以这个函数就没有实现 PutAttrByAttr()
   * 而且我觉得要用到这个功能会比较少
   */
   
  /*
   *
   * add a new line in xml 
   * myMap.insert("id", "ttt");
   * myMap.insert("value", "000");
   * myMap.insert("prompt", "ppp");
   * myMap.insert("msg", "mmm");
   * AddLineByAttribute("errors.error", myMap);
   *  <errors>
   *  <error id="DB_ERROR_EXECUTE" value="1" prompt="操作数据库失败" msg="操作数据库失败" />
   *  <error id="DB_ERROR_COMMAND" value="2" prompt="创建数据库操作指令失败" msg="创建数据库操作指令失败"/>
   *  </errors>
   * new:
   *  <errors>
   *  <error id="DB_ERROR_EXECUTE" value="1" prompt="操作数据库失败" msg="操作数据库失败" />
   *  <error id="DB_ERROR_COMMAND" value="2" prompt="创建数据库操作指令失败" msg="创建数据库操作指令失败"/>
   *  <error id="ttt" value="000" prompt="ppp" msg="mmm"/>
   *  </errors>
   */
  void AddLineByAttr(string path, const map<string, string>& key_value_map);

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

typedef boost::shared_ptr<ParseXmlObj> ParseXmlObjPtr;
#endif /* MQPOOL_INCLUDE_PARSE_XML_H */
