#include "LBPFeatureExtractor.hpp"
#include <opencv2/opencv.hpp>

using namespace cv;

// template <typename _tp>
// void getOriginLBPFeature(cv::Mat src,cv::Mat dst)
// {
//     dst.create(src.rows-2,src.cols-2,CV_8UC1);
//     dst.setTo(0);
//     for(int i=1;i<src.rows-1;i++)
//     {
//         for(int j=1;j<src.cols-1;j++)
//         {
//             _tp center = src.at<_tp>(i,j);
//             unsigned char lbpCode = 0;
//             lbpCode |= (src.at<_tp>(i-1,j-1) > center) << 7;
//             lbpCode |= (src.at<_tp>(i-1,j  ) > center) << 6;
//             lbpCode |= (src.at<_tp>(i-1,j+1) > center) << 5;
//             lbpCode |= (src.at<_tp>(i  ,j+1) > center) << 4;
//             lbpCode |= (src.at<_tp>(i+1,j+1) > center) << 3;
//             lbpCode |= (src.at<_tp>(i+1,j  ) > center) << 2;
//             lbpCode |= (src.at<_tp>(i+1,j-1) > center) << 1;
//             lbpCode |= (src.at<_tp>(i  ,j-1) > center) << 0;
//             dst.at<uchar>(i-1,j-1) = lbpCode;
//         }
//     }
// }

// Mat getLBPH(InputArray _src,int numPatterns,int grid_x,int grid_y,bool normed)
// {
//     Mat src = _src.getMat();
//     int width = src.cols / grid_x;
//     int height = src.rows / grid_y;
//     //定义LBPH的行和列，grid_x*grid_y表示将图像分割成这么些块，numPatterns表示LBP值的模式种类
//     Mat result = Mat::zeros(grid_x * grid_y,numPatterns,CV_32FC1);
//     if(src.empty())
//     {
//         return result.reshape(1,1);
//     }
//     int resultRowIndex = 0;
//     //对图像进行分割，分割成grid_x*grid_y块，grid_x，grid_y默认为8
//     for(int i=0;i<grid_x;i++)
//     {
//         for(int j=0;j<grid_y;j++)
//         {
//             //图像分块
//             Mat src_cell = Mat(src,Range(i*height,(i+1)*height),Range(j*width,(j+1)*width));
//             //计算直方图
//             Mat hist_cell = getLocalRegionLBPH(src_cell,0,(numPatterns-1),true);
//             //将直方图放到result中
//             Mat rowResult = result.row(resultRowIndex);
//             hist_cell.reshape(1,1).convertTo(rowResult,CV_32FC1);
//             resultRowIndex++;
//         }
//     }
//     return result.reshape(1,1);
// }
// //计算一个LBP特征图像块的直方图
// Mat getLocalRegionLBPH(const Mat& src,int minValue,int maxValue,bool normed)
// {
//     //定义存储直方图的矩阵
//     Mat result;
//     //计算得到直方图bin的数目，直方图数组的大小
//     int histSize = maxValue - minValue + 1;
//     //定义直方图每一维的bin的变化范围
//     float range[] = { static_cast<float>(minValue),static_cast<float>(maxValue + 1) };
//     //定义直方图所有bin的变化范围
//     const float* ranges = { range };
//     //计算直方图，src是要计算直方图的图像，1是要计算直方图的图像数目，0是计算直方图所用的图像的通道序号，从0索引
//     //Mat()是要用的掩模，result为输出的直方图，1为输出的直方图的维度，histSize直方图在每一维的变化范围
//     //ranges，所有直方图的变化范围（起点和终点）
//     calcHist(&src,1,0,Mat(),result,1,&histSize,&ranges,true,false);
//     //归一化
//     if(normed)
//     {
//         result /= (int)src.total();
//     }
//     //结果表示成只有1行的矩阵
//     return result.reshape(1,1);
// }