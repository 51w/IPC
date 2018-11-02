#include "mobilefacenet.h"

MobileNetFeatureExtractor::MobileNetFeatureExtractor(const std::string &model_path) {
	std::string param_files = model_path + "/mobilefacenet.param";
	std::string bin_files = model_path + "/mobilefacenet.bin";
	net_.load_param(param_files.c_str());
	net_.load_model(bin_files.c_str());
}

MobileNetFeatureExtractor::~MobileNetFeatureExtractor() {
	net_.clear();
}

void MobileNetFeatureExtractor::getFeature(const cv::Mat& img, std::vector<float>&feature) {
	ncnn::Mat ncnn_img = ncnn::Mat::from_pixels_resize(img.data, ncnn::Mat::PIXEL_BGR2RGB, img.cols, img.rows, 112, 112);
    ncnn::Extractor ex = net_.create_extractor();
    //ex.set_num_threads(2);
    ex.set_light_mode(true);
    ex.input("data", ncnn_img);
    ncnn::Mat out;
    ex.extract("fc1", out);
    feature_.resize(feature_dim_);
    for (int j = 0; j < feature_dim_; j++)
    {
        feature_[j] = out[j];
    }
	feature = feature_;
}

double calculSimilar(std::vector<float>& v1, std::vector<float>& v2)
{
	assert(v1.size() == v2.size());
	double ret = 0.0, mod1 = 0.0, mod2 = 0.0;
	for (std::vector<double>::size_type i = 0; i != v1.size(); ++i)
	{
		ret += v1[i] * v2[i];
		mod1 += v1[i] * v1[i];
		mod2 += v2[i] * v2[i];
	}
	return ret / sqrt(mod1) / sqrt(mod2);
}
