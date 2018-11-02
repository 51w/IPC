#pragma once
#ifndef MOBILEFACENET_H_
#define MOBILEFACENET_H_
#include <string>
#include "net.h"
#include "opencv2/opencv.hpp"

class MobileNetFeatureExtractor {
public:
    MobileNetFeatureExtractor(const std::string &model_path);
	~MobileNetFeatureExtractor();
	void getFeature(const cv::Mat& img, std::vector<float>&feature);
private:
	ncnn::Net net_;
	ncnn::Mat ncnn_img_;
	std::vector<float> feature_;
    int feature_dim_ = 128;
};

double calculSimilar(std::vector<float>& v1, std::vector<float>& v2);


#endif // !MOBILEFACENET_H_