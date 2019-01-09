TARGET = client

OBJS_PATH = objs

#CROSS_COMPILE = arm-hisiv300-linux-
CROSS_COMPILE = 
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc
STRIP = $(CROSS_COMPILE)strip

INC  += -I$(shell pwd)/../ubuntu16/include/BasicUsageEnvironment
INC  += -I$(shell pwd)/../ubuntu16/include/groupsock
INC  += -I$(shell pwd)/../ubuntu16/include/liveMedia
INC  += -I$(shell pwd)/../ubuntu16/include/UsageEnvironment
INC  += -I$(shell pwd)/include
LIB  += -L$(shell pwd)/static -lavcodec -lavutil
LIB  += -L$(shell pwd)/../ubuntu16/lib
LD_FLAGS  = -lpthread -lliveMedia -lBasicUsageEnvironment -lUsageEnvironment -lgroupsock

O_FLAG   = -O2

SRC1  = $(notdir $(wildcard ./*.cpp))
OBJS1 = $(patsubst %.cpp,$(OBJS_PATH)/%.o,$(SRC1))

SRC2  = $(notdir $(wildcard ./*.c))
OBJS2 = $(patsubst %.c,$(OBJS_PATH)/%.o,$(SRC2))

all: BUILD_DIR $(TARGET)

BUILD_DIR:
	@-mkdir -p $(OBJS_PATH)

$(TARGET) : $(OBJS1) $(OBJS2)
	$(CXX) $^ -o $@ $(CFLAGS) $(LIB) $(LD_FLAGS) $(CXX_FLAGS)
	rm -rf $(OBJS_PATH)
 
$(OBJS_PATH)/%.o : ./%.cpp
	$(CXX) -c  $< -o  $@  $(CXX_FLAGS) $(INC)

$(OBJS_PATH)/%.o : ./%.c
	$(CC) -c  $< -o  $@  $(CXX_FLAGS) $(INC)

clean:
	-rm -rf $(OBJS_PATH) $(TARGET)
