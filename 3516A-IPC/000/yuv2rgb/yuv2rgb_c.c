#include "yuv2rgb_c.h"
#include "yuv2rgb.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int nv21_to_rgb_c(unsigned char* rgb, unsigned char const* nv21, int width, int height)
{
	return nv21_to_rgb(rgb, nv21, width, height);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif