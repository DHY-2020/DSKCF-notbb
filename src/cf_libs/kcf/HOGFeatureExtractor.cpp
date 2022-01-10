#include <memory>

#include "HOGFeatureExtractor.hpp"
#include "gradientMex.hpp"
#include <iostream>
// #include "LBPFeatureExtractor.hpp"
using namespace std;
using namespace cv;

// template <typename _tp>
cv::Mat getOriginLBPFeature(cv::Mat src)
{
	Mat dst;
    dst.create(src.rows,src.cols,CV_8UC1);
	// cout<<"src:"<<src.size()<<endl;
    dst.setTo(0);
    for(int i=1;i<src.rows-1;i++)
    {
        for(int j=1;j<src.cols-1;j++)
        {
            uchar center = src.at<uchar>(i,j);
			// cout<<"center:"<<center<<endl;
            unsigned char lbpCode = 0;
            lbpCode |= (src.at<uchar>(i-1,j-1) > center) << 7;
            lbpCode |= (src.at<uchar>(i-1,j  ) > center) << 6;
            lbpCode |= (src.at<uchar>(i-1,j+1) > center) << 5;
            lbpCode |= (src.at<uchar>(i  ,j+1) > center) << 4;
            lbpCode |= (src.at<uchar>(i+1,j+1) > center) << 3;
            lbpCode |= (src.at<uchar>(i+1,j  ) > center) << 2;
            lbpCode |= (src.at<uchar>(i+1,j-1) > center) << 1;
            lbpCode |= (src.at<uchar>(i  ,j-1) > center) << 0;
            dst.at<uchar>(i,j) = lbpCode;
        }
    }
	// cout<<"dst:"<<dst<<endl;
	return dst;
}

//计算一个LBP特征图像块的直方图
Mat getLocalRegionLBPH(const Mat& src,int minValue,int maxValue,bool normed)
{
    //定义存储直方图的矩阵
    Mat result;
    //计算得到直方图bin的数目，直方图数组的大小
    int histSize = maxValue - minValue + 1;
    //定义直方图每一维的bin的变化范围
    float range[] = { static_cast<float>(minValue),static_cast<float>(maxValue + 1) };
    //定义直方图所有bin的变化范围
    const float* ranges = { range };
    //计算直方图，src是要计算直方图的图像，1是要计算直方图的图像数目，0是计算直方图所用的图像的通道序号，从0索引
    //Mat()是要用的掩模，result为输出的直方图，1为输出的直方图的维度，histSize直方图在每一维的变化范围
    //ranges，所有直方图的变化范围（起点和终点）
    calcHist(&src,1,0,Mat(),result,1,&histSize,&ranges,true,false);
    //归一化
    if(normed)
    {
        result /= (int)src.total();
    }
    //结果表示成只有1行的矩阵
    return result.reshape(1,1);
}


Mat getLBPH(cv::Mat src,int numPatterns,int grid_x,int grid_y,bool normed)
{
    // Mat src = _src.getMat();
    int width = src.cols / grid_x;
    int height = src.rows / grid_y;
    //定义LBPH的行和列，grid_x*grid_y表示将图像分割成这么些块，numPatterns表示LBP值的模式种类
    Mat result = Mat::zeros(grid_x * grid_y,numPatterns,CV_32FC1);
    if(src.empty())
    {
        return result.reshape(1,1);
    }
    int resultRowIndex = 0;
    //对图像进行分割，分割成grid_x*grid_y块，grid_x，grid_y默认为8
    for(int i=0;i<grid_x;i++)
    {
        for(int j=0;j<grid_y;j++)
        {
            //图像分块
            Mat src_cell = Mat(src,Range(i*height,(i+1)*height),Range(j*width,(j+1)*width));
            //计算直方图
            Mat hist_cell = getLocalRegionLBPH(src_cell,0,(numPatterns-1),true);
            //将直方图放到result中
            Mat rowResult = result.row(resultRowIndex);
            hist_cell.reshape(1,1).convertTo(rowResult,CV_32FC1);
            resultRowIndex++;
        }
    }
    return result.reshape(1,80);
}

HOGFeatureExtractor::HOGFeatureExtractor()
{
	this->m_cellSize = 4;
}

HOGFeatureExtractor::~HOGFeatureExtractor()
{
}

std::shared_ptr< FC > HOGFeatureExtractor::getFeatures( const cv::Mat & image,const cv::Rect_< double > & boundingBox ) const
{
	cv::Mat image2 = image.clone(); 
	// if (image2.channels()<3)
	// {
	// 	image2 = (image2/65535)*255; //将图像范围转到0-255，结果对尺度变换更好

	// 	cv::Mat LBF_feature;
	// 	LBF_feature = getOriginLBPFeature(image2);
	// 	cv::Mat result;
	// 	result = getLBPH(LBF_feature, 256, 5, 5, true);


	// 	// cout<<"LBF_feature:"<<LBF_feature.size()<<endl;
	// 	cout<<"result:"<<result.size()<<endl;

	// }
	cv::Mat patch;
	if( getSubWindow< double >( image2, patch, boundingBox.size(), centerPoint( boundingBox ) ) )
	{
	  cv::Mat patchResizedFloat;
	  cv::Mat patchLBPResizedFloat;
	  cv::Mat patchLBP;
	  patch.copyTo(patchLBP);
	  if (patchLBP.channels()>1)
	  {
		  cvtColor(patchLBP, patchLBP, COLOR_BGR2GRAY);
		//   patchLBP = patchLBP/255;
	  }
	  else
	  {
		  patchLBP = (patchLBP/65535)*255;
		//   patchLBP.convertTo(patchLBP, CV_8U);
	  }
	  if (patch.channels()<3)
	  {
		  patch = (patch/65535)*255;
		//   patch.convertTo(patch, CV_8U);
		//   cvtColor(patch, patch, COLOR_BGR2GRAY);
	  }
	//   else
	//   {
	// 	//   patch = patch/255;
	// 	//   cvtColor(patch, patch, COLOR_BGR2GRAY);
	//   }
	//   GaussianBlur(patchLBP, patchLBP, Size(3,3),0,0);
	//   patchLBP = (patchLBP/65535);
	  cv::Mat LBF_feature;
	//   Canny(patchLBP, LBF_feature, 128, 255, 3);
	//   cout<<"LBF_feature:"<<LBF_feature<<endl;
	//   patchLBP = (patchLBP/65535)*255;
	  LBF_feature = getOriginLBPFeature(patchLBP);
	//   cout<<"LBF_feature_size:"<<LBF_feature.size()<<endl;
	//   cv::Mat result;
	//   result = getLBPH(LBF_feature, 256, 5, 5, true);



	  patch.convertTo(patchResizedFloat, CV_32FC(3));
	  LBF_feature.convertTo(patchLBPResizedFloat, CV_32FC(3));
	//   patch.convertTo(patchResizedFloat, CV_8SC(3));
    //   std::cout<<"patchLBPResizedFloat:"<<patchLBPResizedFloat<<endl;
	//   patchResizedFloat = patchResizedFloat/255;
	//   patchLBPResizedFloat = patchLBPResizedFloat/255;
	  auto features = std::make_shared< FC >();
	  auto featureslbp = std::make_shared< FC >();
	//   featureslbp = result;
	//   cout<<"end1"<<endl;
	  piotr::cvFhog< double, FC >(patchResizedFloat, features, this->m_cellSize);
	  piotr::cvFhog< double, FC >(patchLBPResizedFloat, featureslbp, this->m_cellSize);
	  features = FC::concatFeatures(features, featureslbp);//直接将LBP+HOG可以得到和RGB一样的效果，不用做任何归一化

	//   cout<<"feature:"<<features.<<endl;
	  	 
		return features;
	}
	else
	{
		std::cerr << "Error : HOGFeatureExtractor::getFeatures : getSubWindow failed!" << std::endl;
	}

	return nullptr;
}
