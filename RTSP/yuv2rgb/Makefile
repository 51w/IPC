CROSS_COMPILE = arm-hisiv300-linux-
CXX   = $(CROSS_COMPILE)g++
CC    = $(CROSS_COMPILE)gcc
AR    = $(CROSS_COMPILE)ar

XX_FLAGS += -DARM_NEON_ENABLE -flax-vector-conversions
XX_FLAGS += -mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -ffunction-sections

all:
	$(CXX) yuv2rgb.cpp yuv2rgb_c.c -shared -fPIC $(XX_FLAGS)  -o libyuv2rgb.so
