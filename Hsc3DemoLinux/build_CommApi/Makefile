CFLAGS = -g -std=c++0x
target = CommApiTraining
INCLUDE =  -I./  -I../Hsc3Api/include -I/usr/local/include/boost
lib = -L../Hsc3Api/lib  -lpthread  -lm -lCommApi -lHsc3Api -lLogApi -lboost_system -lboost_thread -lboost_chrono
dirlib = -L ./

CFLAGS += -DALIGNATTRIB= -DCDECL= -DHANDLE_WIN32_PRAGMA -DDLL_DECL= -D_REENTRANT= -DHUGEPTR= -D_LINUX_=
CFLAGS += -O2 -Wall -Wno-unused
CFLAGS += $(INCLUDE)
CFLAGS += -DLINUX
CFLAGS += -fPIC  -Xlinker -zmuldefs

src = CommApiTraining.cpp
objs = $(patsubst %.cpp,%.o,$(src))

$(target) : $(objs)
	g++ $(CFLAGS)  $(objs) $(dirlib) $(lib) -o $(target)  $(INCLUDE)

%.o: %.cpp
	g++ $(CFLAGS)   $(INCLUDE) $(lib) $(dirlib) -c $< -o $@

.PHONY:clean
clean:
	rm -f $(target)  $(objs)