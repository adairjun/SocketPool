#ifndef MQPOOL_INCLUDE_LOGOBJ_H
#define MQPOOL_INCLUDE_LOGOBJ_H

#include <string>

using std::string;

class LogObj {
 public:
  //日志的级别
  typedef enum {
	TRACE,            //跟踪
	DEBUG,            //除错
	INFO,             //信息
	WARN,             //警告
	ERROR,            //错误
	FATAL,            //致命
	NUM_LOG_LEVELS    //日志级别的数量
  } LogLevel;

  /*
   * 转换函数，使用到了表驱动法
   */
  static LogLevel LogLevelConvert(string pLevel);
  static string LogLevelConvert(LogLevel eLevel);

 public:
  explicit LogObj();
  explicit LogObj(const string& logName, const string& savePath);

  /*
   * 这个构造函数的参数传递的是__FILE__,__LINE__这两个宏和日志级别
   */
  explicit LogObj(const char* FILE, int LINE, LogLevel logLevel);

  /*
   * 在我的LOG宏当中选用的是这个构造函数，而其他三个构造函数就是在我最初写代码的时候用到的
   */
  explicit LogObj(const string& logName,
		          const string& savePath,
				  const char* FILE,
				  int LINE,
				  LogLevel logLevel);

  LogObj(const LogObj&) = delete;
  LogObj& operator=(const LogObj&) = delete;

  /*
   * 在对象析构的时候调用WriteLog函数，把buffer_当中的数据写入到日志文件
   */
  virtual ~LogObj();

  void Dump() const;

 public:
  string GetLogName() const;

  string GetSavePath() const;

  void SetLogName(const string& logName);

  void SetSavePath(const string& savePath);

  string GetCurrentTime(int flag) const;

 public:
  //这一组函数是操作buffer_的

  /*
   * 往buffer_后面追加buf的len长度的字符
   */
  void Append(const char* buf, unsigned int len);

  const char* GetBuffer() const;

  /*
   * 把buffer_清空，全置为0
   */
  void ResetBuffer();

 public:
  //这里是仿照陈硕的logSteam写的，但是我并没有像他那样重载那么多的<<操作符
  LogObj& operator<<(int v);
  LogObj& operator<<(unsigned int v);
  LogObj& operator<<(float v);
  LogObj& operator<<(double v);
  LogObj& operator<<(const char* v);
  LogObj& operator<<(const string& v);

 public:
  /*
   * 只是简单地把buffer_当中的数据写入到日志文件，后续可以继续扩展
   */
  void WriteLog();


 private:
  string logName_;                        //日志名称
  string savePath_;                       //日志保存目录
  LogLevel logLevel_;                     //日志类型

 private:
  char buffer_[2048];
};

//这里也是借鉴了陈硕的代码，使用临时匿名LogObj对象
//因为临时匿名对象是一使用完就马上调用析构函数销毁。
//而C++对于栈中的具名对象，先创建的后销毁。
//这就使得后创建的Logger对象先于先创建的Logger对象销毁。
//即先调用析构函数将日志输出，这就会使得日志内容反序（具体说是一个由{}包括的块中反序）。
//使用临时匿名Logger对象的效果就是：LOG(ERROR)这行代码不仅仅包含日志内容，还会马上把日志输出。

#define G_LOGNAME "undefined_.log"
#define G_SAVEPATH "../log"

#define LOG(level) LogObj(G_LOGNAME, G_SAVEPATH, __FILE__, __LINE__, LogObj::level)

/*
 * 使用方式:LOG(ERROR) << "this is error";
 * 但是这里不能够使用endl，比如说LOG(ERROR) << "this is error" << endl; 不能通过编译
 * 还有就是不能在这里savePath_的末尾不能有/符号，这是因为我在WriteLog函数当中用了/导致的
 */

#endif /* MQPOOL_INCLUDE_LOGOBJ_H */
