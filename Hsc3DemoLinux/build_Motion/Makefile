CFLAGS = -g -std=c++0x
target = TrainingJog TrainingJogSingle TrainingJogHome
INCLUDE =  -I./  -I../Hsc3Api/include -I/usr/local/include/boost
lib = -L../Hsc3Api/lib  -lpthread  -lm -lCommApi -lHsc3Api -lLogApi -lboost_system -lboost_thread -lboost_chrono
dirlib = -L ./

CFLAGS += -DALIGNATTRIB= -DCDECL= -DHANDLE_WIN32_PRAGMA -DDLL_DECL= -D_REENTRANT= -DHUGEPTR= -D_LINUX_=
CFLAGS += -O2 -Wall -Wno-unused
CFLAGS += $(INCLUDE)
CFLAGS += -DLINUX
CFLAGS += -fPIC  -Xlinker -zmuldefs

src = TrainingJog.cpp TrainingJogSingle.cpp TrainingJogHome.cpp
objs = $(patsubst %.cpp,%.o,$(src))

all: $(target)

TrainingJog : TrainingJog.o
	g++ $(CFLAGS)  TrainingJog.o $(dirlib) $(lib) -o TrainingJog  $(INCLUDE)

TrainingJogSingle : TrainingJogSingle.o
	g++ $(CFLAGS)  TrainingJogSingle.o $(dirlib) $(lib) -o TrainingJogSingle  $(INCLUDE)

TrainingJogHome : TrainingJogHome.o
	g++ $(CFLAGS)  TrainingJogHome.o $(dirlib) $(lib) -o TrainingJogHome  $(INCLUDE)

%.o: %.cpp
	g++ $(CFLAGS)   $(INCLUDE) $(lib) $(dirlib) -c $< -o $@

.PHONY:clean
clean:
	rm -f $(target)  $(objs)