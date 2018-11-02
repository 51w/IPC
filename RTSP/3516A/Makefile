TARGET = rtsp

OBJS_PATH = objs

CROSS_COMPILE = arm-hisiv300-linux-
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

#PLAT = ubuntu16
PLAT = hisiv300

INC  += -I$(shell pwd)/../bufpool
LIB  += -L$(shell pwd)/../bufpool -lbufpool

INC  += -I$(shell pwd)/$(PLAT)/include/BasicUsageEnvironment
INC  += -I$(shell pwd)/$(PLAT)/include/groupsock
INC  += -I$(shell pwd)/$(PLAT)/include/liveMedia
INC  += -I$(shell pwd)/$(PLAT)/include/UsageEnvironment
LIB  += -L$(shell pwd)/$(PLAT)/lib
LD_FLAGS  = -lpthread -lliveMedia -lBasicUsageEnvironment -lUsageEnvironment -lgroupsock

O_FLAG   = -O2

SRC1  = $(notdir $(wildcard ./src/*.cpp))
OBJS1 = $(patsubst %.cpp,$(OBJS_PATH)/%.o,$(SRC1))

all: BUILD_DIR $(TARGET)

BUILD_DIR:
	@-mkdir -p $(OBJS_PATH)

$(TARGET) : $(OBJS1)
	$(CXX) $^ -o $@ $(CFLAGS) $(LIB) $(LD_FLAGS) $(CXX_FLAGS)
 
$(OBJS_PATH)/%.o : ./src/%.cpp
	$(CXX) -c  $< -o  $@  $(CXX_FLAGS) $(INC) -lpthread


clean:
	-rm -rf $(OBJS_PATH) $(TARGET)
