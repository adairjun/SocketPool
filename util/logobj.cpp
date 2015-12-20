#include "SocketPool/logobj.h"
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

typedef struct {
  LogObj::LogLevel eLevel;
  string pName;
} LevelName;


// 这是代码大全当中的表驱动法
static LevelName LEVEL_NAME_MAPPING[] = {
  {LogObj::TRACE, "TRACE"},
  {LogObj::DEBUG, "DEBUG"},
  {LogObj::INFO, "INFO"},
  {LogObj::WARN, "WARN"},
  {LogObj::ERROR, "ERROR"},
  {LogObj::FATAL, "FATAL"},
  {LogObj::NUM_LOG_LEVELS, "NUM_LOG_LEVELS"}
};


LogObj::LogLevel LogObj::LogLevelConvert(string pLevel) {
  for (unsigned i = 0;i < sizeof(LEVEL_NAME_MAPPING)/sizeof(LevelName); ++i) {
  if ( LEVEL_NAME_MAPPING[i].pName == pLevel) {
	  return LEVEL_NAME_MAPPING[i].eLevel;
	}
  }
  return LogObj::NUM_LOG_LEVELS;
}

string LogObj::LogLevelConvert(LogObj::LogLevel eLevel) {
  for (unsigned i = 0; i < sizeof(LEVEL_NAME_MAPPING)/sizeof(LevelName); ++i) {
	if ( LEVEL_NAME_MAPPING[i].eLevel == eLevel ) {
	  return LEVEL_NAME_MAPPING[i].pName;
	}
  }
  return "";
}

/*
LogObj::LogObj()
    : logName_("undefined_.log"),
	  savePath_("../log"),
	  logLevel_(LogObj::NUM_LOG_LEVELS) {
	//把当前的日期加入到日志的名字当中
	logName_ += GetCurrentTime(0);
	memset(buffer_, 0, 2048);
}

LogObj::LogObj(const string& logName, const string& savePath, LogLevel logLevel)
    : logName_(logName),
      savePath_(savePath),
	  logLevel_(logLevel) {
	logName_ += GetCurrentTime(0);
	memset(buffer_, 0, 2048);
}

LogObj::LogObj(const char* FILE, int LINE, LogLevel logLevel)
    : logName_("undefined_.log"),
	  savePath_("../log/MQPool") {
	logName_ += GetCurrentTime(0);
	memset(buffer_, 0, 2048);
	//获取到当前的时间，再把__FILE,__LINE__ 还有日志级别先写入到buffer_当中
	string level = LogObj::LogLevelConvert(logLevel);
	string now = GetCurrentTime(1);
    sprintf(buffer_,
    		"LogLevel[%s]-----Time:[%s]\nFILE:[%s]-----LINE:[%d]\n",
			level.c_str(),
			now.c_str(),
			FILE,
			LINE);
}
*/

LogObj::LogObj(const string& logName,
		          const string& savePath,
				  LogLevel logLevel,
				  const char* FILE,
				  int LINE)
    : logName_(logName),
      savePath_(savePath),
	  logLevel_(logLevel) {
	logName_ += GetCurrentTime(0);
	memset(buffer_, 0, 2048);
	//获取到当前的时间，再把__FILE,__LINE__ 还有日志级别先写入到buffer_当中
	string level = LogObj::LogLevelConvert(logLevel);
	string now = GetCurrentTime(1);
	sprintf(buffer_,
	    	"LogLevel[%s]-----Time:[%s]\nFILE:[%s]-----LINE:[%d]\n",
			level.c_str(),
			now.c_str(),
			FILE,
			LINE);
}

LogObj::~LogObj() {
  WriteLog();
}

void LogObj::Dump() const{
  printf("\n=====LogObj Dump START ========== \n");
  printf("logName_=%s ", logName_.c_str());
  printf("savePath_=%s ", savePath_.c_str());
  printf("buffer_=%s ", buffer_);
  printf("\n===LogObj DUMP END ============\n");
}

string LogObj::GetLogName() const {
  return logName_;
}

string LogObj::GetSavePath() const {
  return savePath_;
}

void LogObj::SetLogName(const string& logName) {
  logName_ = logName;
}

void LogObj::SetSavePath(const string& savePath) {
  savePath_ = savePath;
}

string LogObj::GetCurrentTime(int flag) const {
  string format = "%Y-%m-%d %H:%M:%S";
  // 如果flag设为0的话，那么旧仅仅获取当前的日期，后面的小时，分钟，秒都不用
  if (flag == 0){
	format = "%Y%m%d";
  }
  time_t now;
  time(&now);
  struct tm nowtm;
  localtime_r(&now, &nowtm);

  char ftime[30];
  size_t len = strftime((char *)&ftime, sizeof(ftime)-1, format.c_str(), &nowtm);
  return std::string((char *)&ftime, len+1); //len+1 to include the last '\0' character
}

void LogObj::Append(const char* buf, unsigned int len) {
  if (strlen(buffer_) + len <= sizeof(buffer_)) {
    memcpy(buffer_ + strlen(buffer_), buf, len);
  } else {
    //TODO cerr<< "no enough space in buffer"
  }
}

const char* LogObj::GetBuffer() const {
  return buffer_;
}

void LogObj::ResetBuffer() {
  memset(buffer_, 0, sizeof(buffer_));
}

LogObj& LogObj::operator <<(int v) {
  char temp[32];
  sprintf(temp,"%d", v);
  *this << temp;
  return *this;
}

LogObj& LogObj::operator <<(unsigned int v) {
  //由于是unsigned int，使用64的长度
  char temp[64];
  sprintf(temp,"%d", v);
  *this << temp;
  return *this;
}

LogObj& LogObj::operator <<(float v) {
  *this << static_cast<double>(v);
  return *this;
}

LogObj& LogObj::operator <<(double v) {
  char temp[128];
  //这里默认仅仅保留三位小数
  sprintf(temp,"%.3lf", v);
  *this << temp;
  return *this;
}

LogObj& LogObj::operator <<(const char* v) {
  if (v) {
    Append(v, strlen(v));
  } else {
    Append("(null)", 6);
  }
  return *this;
}

LogObj& LogObj::operator<<(const string& v) {
  Append(v.c_str(), v.size());
  return *this;
}

void LogObj::WriteLog() {
  //由于这里需要将savePath_和logName组合起来，我默认的savePath_的目录的结尾是没有/符号的
  int fd = open((savePath_ + "/" + logName_).c_str(), O_CREAT|O_RDWR|O_APPEND|O_LARGEFILE, 00640);
  dprintf(fd, "%s\n", buffer_);
  close(fd);
}
