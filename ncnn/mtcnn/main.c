#include "mtcnn.h"
//#include <opencv2/opencv.hpp>
#include <sys/time.h>
//using namespace cv;

double get_current_time()
{
    struct timeval tv;
	
    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

int ncnn_detect00(unsigned char* buff, int width, int height, int *num, int *box)
{
	MTCNN mtcnn("../ncnn/model/");
	
	//FILE* pfd = fopen("zzz.yuv", "rb");
	//unsigned char buff[480*270*3]; 
	//fread(buff, 480*270*3, 1, pfd);
	ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(buff, ncnn::Mat::PIXEL_RGB, width, height);
	

	//cv::Mat image;
	//image = cv::imread("./3.jpg");
	
	//ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(image.data, ncnn::Mat::PIXEL_BGR2RGB, image.cols, image.rows);
	std::vector<Bbox> finalBbox;
	
	//double begin = get_current_time();
	mtcnn.detect(ncnn_img, finalBbox);
	//double end = get_current_time();
	//std::cout<<"FD cost: "<< end - begin << "ms" << std::endl;

	
	*num = finalBbox.size();
	for (int i = 0; i < finalBbox.size(); i++) 
	{
		box[i*4 + 0] = finalBbox[i].x1;
		box[i*4 + 1] = finalBbox[i].y1;
		box[i*4 + 2] = finalBbox[i].x2;
		box[i*4 + 3] = finalBbox[i].y2;
		//std::cout <<finalBbox[i].x1 << " " <<
		//			finalBbox[i].y1 << " " <<
		//			finalBbox[i].x2 << " " <<
		//			finalBbox[i].y2 << " " << std::endl;
	}
	//fclose(pfd);
	
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

int ncnn_detect(unsigned char* buff, int width, int height, int *num, int *box)
{
	return ncnn_detect00(buff, width, height, num, box);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif