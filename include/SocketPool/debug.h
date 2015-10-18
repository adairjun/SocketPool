// Copyright 
// License
// Author: adairjun
// This is used to detect bug

#ifndef DBPOOL_INCLUDE_DEBUG_H_
#define DBPOOL_INCLUDE_DEBUG_H_

#include <iostream>
// 这个BUG_FILE()宏是用来打印错误信息,使用编译器内置宏__FILE__和__LINE__来定位错误位置
#define BUG_FILE() \
{ \
    std::cerr << " FILE(" << __FILE__ << "), LINE(" << __LINE__ << ")" << std::endl; \
}

#endif /* DBPOOL_INCLUDE_DEBUG_H_ */
