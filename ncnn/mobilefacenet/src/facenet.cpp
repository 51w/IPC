#include <iostream>
#include "mobilefacenet.h"
#include "mtcnn.h"

int main(int argc, char** argv)
{
	MobileNetFeatureExtractor *pfe = new MobileNetFeatureExtractor("../models");
	std::vector<float> feature1;
    std::vector<float> feature2;
	std::vector<float> feature3;
	cv::Mat img1 = cv::imread("imgs/cut.jpg");
	pfe->getFeature(img1, feature1);
	cv::Mat img2 = cv::imread("imgs/cut.jpg");
	pfe->getFeature(img2, feature2);
	cv::Mat img3 = cv::imread("imgs/cut.jpg");
	pfe->getFeature(img3, feature3);
	//double ss = calculSimilar(feature2, feature1);
	//std::cout << ss << std::endl;
    
	//test_detection();
    //testvalidation(argv[1], argv[2]);
	
	MTCNN mtcnn("../models");
	cv::Mat image = cv::imread(argv[1]);
	
	ncnn::Mat ncnn_img = ncnn::Mat::from_pixels(image.data, ncnn::Mat::PIXEL_BGR2RGB, image.cols, image.rows);
	std::vector<Bbox> finalBbox;
	mtcnn.detect(ncnn_img, finalBbox);
	
	const int num_box = finalBbox.size();
	std::vector<cv::Rect> bbox;
	bbox.resize(num_box);
	for (int i = 0; i < num_box; i++) {
		bbox[i] = cv::Rect(finalBbox[i].x1, finalBbox[i].y1, finalBbox[i].x2 - finalBbox[i].x1 + 1, finalBbox[i].y2 - finalBbox[i].y1 + 1);
	}
	
	std::cout << "============" << num_box << std::endl;
	
	// 身份认证
	for (vector<cv::Rect>::iterator it = bbox.begin(); it != bbox.end(); it++)
	{
		cv::Rect object = (*it);
		rectangle(image, object, cv::Scalar(0, 0, 255), 2, 8, 0);
		
		cv::Mat cut = image(object);
		//cv::imwrite("cut.jpg", cut);
		
		std::vector<float> feature;
		pfe->getFeature(cut, feature);
		
		double similarity = 0.0f;
		similarity += calculSimilar(feature, feature1);
		std::cout << similarity << std::endl;
		// similarity += calculSimilar(feature, feature2);
		// std::cout << similarity << std::endl;
		// similarity += calculSimilar(feature, feature3);
		// std::cout << similarity << std::endl;
		
		if(similarity > 0.5)
		{
			std::string label = "wang";
			int baseLine = 0;
            cv::Size label_size = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
			
			rectangle(image, (*it), cv::Scalar(0, 0, 255), 2, 8, 0);
			cv::rectangle(image, cv::Rect(cv::Point(object.x, object.y- label_size.height),
                cv::Size(label_size.width, label_size.height + baseLine)), cv::Scalar(255, 255, 255), CV_FILLED);
            cv::putText(image, label, cv::Point(object.x, object.y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
		}
	}
	cv::imwrite("result.jpg", image);
	
	return 0;
}