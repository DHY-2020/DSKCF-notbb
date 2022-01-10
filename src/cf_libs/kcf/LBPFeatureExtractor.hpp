#ifndef _LBPFEATURES_HPP_
#define _LBPFEATURES_HPP_
#include <opencv2/opencv.hpp>
using namespace cv;

void getOriginLBPFeature(cv::Mat src,cv::Mat dst);
Mat getLBPH(InputArray _src,int numPatterns,int grid_x,int grid_y,bool normed);
Mat getLocalRegionLBPH(const Mat& src,int minValue,int maxValue,bool normed);

#endif
