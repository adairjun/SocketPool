# # Use 'make V=1' to see the full commands
#使用安静模式可以不看gcc,g++的参数
CC := gcc 
CXX := g++
AR := ar

LIBRARY := libsocketpool.a
SHARED := libsocketpool.so

INCLUDE := -I/usr/local/include -I./include
# -lgtest 一定要放在 -lpthread 前面
LIBS := -L/usr/local/lib -lgtest -ltheron -levent -lpthread 

CFLAGS := 
CPPFLAGS := -std=c++11 -O2 -g -fPIC -DNDEBUG
SHARED_LDFLAGS := -shared -fPIC -Wl,-soname,${SHARED}

LIBCFILES := $(wildcard ./util/*.c)
LIBCPPFILES := $(wildcard ./util/*.cc ./util/*.cpp)
LIBOBJECTS := $(addsuffix .o, $(basename $(LIBCFILES)) $(basename $(LIBCPPFILES)))

CFILES := $(wildcard ./gtest/*.c)
CPPFILES := $(wildcard ./gtest/*.cc ./gtest/*.cpp)
OBJECTS := $(addsuffix .o, $(basename $(CFILES)) $(basename $(CPPFILES)))
TARGETS := $(basename $(OBJECTS))

# 安静模式的核心代码
ifeq ("$(origin V)", "command line")
   BUILD_VERBOSE = $(V)
endif
ifndef BUILD_VERBOSE
   BUILD_VERBOSE = 0
endif

ifeq ($(BUILD_VERBOSE),0)
	QUIET_CC        = @echo '   ' CC $@;
	QUIET_CXX       = @echo '   ' CXX $@;
	QUIET_LINK      = @echo '   ' LINK $@;
	QUIET_AR        = @echo '   ' AR $@;
endif

all: $(LIBRARY) $(SHARED) 
	@echo "--------------------------make successful-----------------------"

check: all $(TARGETS)

$(LIBRARY): $(LIBOBJECTS)
	-rm -rf $@
	$(QUIET_AR)$(AR) crv $@ $(LIBOBJECTS)

$(SHARED):
	$(QUIET_CXX)$(CXX) $(SHARED_LDFLAGS) -o $@ $(LIBOBJECTS) $(LIBS)

$(TARGETS): $(OBJECTS)
	$(QUIET_LINK)$(CXX) -DNDEBUG -o $@ $(addsuffix .o, $@) $(LIBS) -L. -lsocketpool

#下面的Makefile其实只是为了使用安静模式而已,如果将下面的代码去掉的话也能编译成功,因为默认的make规则将被执行
./util/%.o:./util/%.c
	$(QUIET_CC)$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

./util/%.o:./util/%.cc
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

./util/%.o:./util/%.cpp 
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

./gtest/%.o:./gtest/%.c
	$(QUIET_CC)$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

./gtest/%.o:./gtest/%.cc
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

./gtest/%.o:./gtest/%.cpp 
	$(QUIET_CXX)$(CXX) $(INCLUDE) $(CPPFLAGS) -c -o $@ $<

.PHONY:clean install

clean:
	-rm -f ./util/*.o ./gtest/*.o $(LIBRARY) $(SHARED) $(TARGETS)
	@echo "--------------------------make clean-----------------------"

install:
	cp -r ./include/SocketPool /usr/local/include
	cp $(LIBRARY) $(SHARED) /usr/local/lib 
