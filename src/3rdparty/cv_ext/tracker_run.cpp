/*M///////////////////////////////////////////////////////////////////////////////////////
//
// IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
// By downloading, copying, installing or using the software you agree to this license.
// If you do not agree to this license, do not download, install,
// copy or use the software.
//
// License Agreement
// For Open Source Computer Vision Library
// (3-clause BSD License)
//
// Copyright (C) 2000-2015, Intel Corporation, all rights reserved.
// Copyright (C) 2009-2011, Willow Garage Inc., all rights reserved.
// Copyright (C) 2009-2015, NVIDIA Corporation, all rights reserved.
// Copyright (C) 2010-2013, Advanced Micro Devices, Inc., all rights reserved.
// Copyright (C) 2015, OpenCV Foundation, all rights reserved.
// Copyright (C) 2015, Itseez Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the names of the copyright holders nor the names of the contributors
//   may be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall copyright holders or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//M*/

/*
// Original file: https://github.com/Itseez/opencv_contrib/blob/292b8fa6aa403fb7ad6d2afadf4484e39d8ca2f1/modules/tracking/samples/tracker.cpp
// Modified by Klaus Haag file: https://github.com/klahaag/cf_tracking/blob/master/src/3rdparty/cv_ext/tracker_run.cpp
// + Authors: Jake Hall, Massimo Camplan, Sion Hannuna
// * Add a variety of additional features to visualize tracker, save results according to RGBD dataset (see details below) and to save processing
//   time as in the DS-KCF paper
//  Princeton RGBD data: Shuran Song and Jianxiong Xiao. Tracking Revisited using RGBD Camera: Baseline and Benchmark. 2013.
*/




#include "tracker_run.hpp"

#include <iostream>
#include <ctype.h>
#include <math_helper.hpp>
#include <numeric>

#include "init_box_selector.hpp"
#include "cf_tracker.hpp"

using namespace cv;
using namespace std;
using namespace TCLAP;

#define SCALE 2

TrackerRun::TrackerRun(string windowTitle) :
  _cmd(_windowTitle.c_str(), ' ', "0.1")
{
  _isPaused = false;
  _isStep = false;
  _exit = false;
  _hasInitBox = false;
  _isTrackerInitialzed = false;
  _targetOnFrame = false;
  _updateAtPos = false;
  _windowTitle = windowTitle;
  _imageIndex = 0;
  _debug = 0;
  _tracker = 0;
  _paras = Parameters();
  _frameIdx = 0;
}

TrackerRun::~TrackerRun()
{
  if (_resultsFile.is_open())
  {
    _resultsFile.close();
  }

  if (_resultsFileTime.is_open())
  {
    _resultsFileTime.close();
  }
  
  if (_tracker)
  {
    delete _tracker;
    _tracker = 0;
  }

  std::cout << "Frame,Time" << std::endl;

  for( int i = 0; i < this->frameTime.size(); i++ )
  {
	  std::cout << i << "," << this->frameTime[ i ] << std::endl;
  }
  
  std::cout << "min," << *std::min_element( this->frameTime.begin(), this->frameTime.end() ) << std::endl;
  std::cout << "min," << *std::max_element( this->frameTime.begin(), this->frameTime.end() ) << std::endl;
  std::cout << "mean," << std::accumulate( this->frameTime.begin(), this->frameTime.end(), 0.0 ) / static_cast< double >( this->frameTime.size() ) << std::endl;
}

Parameters TrackerRun::parseCmdArgs(int argc, const char** argv)
{
  Parameters paras;

  try{
    ValueArg<int> deviceIdArg("c", "cam", "Camera device id", false, 0, "integer", _cmd);
    ValueArg<string> seqPathArg("s", "seq", "Path to sequence", false, "", "path", _cmd);
    // ValueArg<string> seq2PathArg("ss", "seq2", "Path to sequence2", false, "", "path", _cmd);
    // ValueArg<string> expansionArgrgb("ii", "image_name_expansion2", "image name expansion (only necessary for image sequences) ie. /%.05d.jpg", false, "", "string", _cmd);
    // ValueArg<string> depth2PathArg("dd", "depth_sequence2", "depth_sequence Path to sequence2", false, "", "path", _cmd);
    // ValueArg<string> expansionArgdepth2("d2", "depth_name_expansion2", "image name expansion (only necessary for image sequences) ie. /%.05d.jpg", false, "", "string", _cmd);
    ValueArg<string> expansionArg("i", "image_name_expansion", "image name expansion (only necessary for image sequences) ie. /%.05d.jpg", false, "", "string", _cmd);
    ValueArg<string> initBbArg("b", "box", "Init Bounding Box", false, "-1,-1,-1,-1", "x,y,w,h", _cmd);
    SwitchArg noShowOutputSw("n", "no-show", "Don't show video", _cmd, false);
    ValueArg<string> outputPathArg("o", "out", "Path to output file", false, "", "file path", _cmd);
    ValueArg<string> imgExportPathArg("e", "export", "Path to output folder where the images will be saved with BB", false, "", "folder path", _cmd);
    SwitchArg pausedSw("p", "paused", "Start paused", _cmd, false);
    SwitchArg repeatSw("r", "repeat", "endless loop the same sequence", _cmd, false);
    ValueArg<int> startFrameArg("", "start_frame", "starting frame idx (starting at 1 for the first frame)", false, 1, "integer", _cmd);
    SwitchArg dummySequenceSw("", "mock_sequence", "Instead of processing a regular sequence, a dummy sequence is used to evaluate run time performance.", _cmd, false);
    SwitchArg dummySequenceSwDepth("", "depth_mock_sequence", "Instead of processing a regular sequence, a dummy sequence is used to evaluate run time performance.", _cmd, false);
    ValueArg<int> deviceIdArgDepth( "", "depth_cam", "Depth device id", false, 0, "integer", _cmd );
    ValueArg<string> seqPathArgDepth( "", "depth_sequence", "Path to depth sequence", false, "", "path", _cmd );
    ValueArg<string> expansionArgDepth( "", "depth_image_name_expansion", "depth image name expansion (only necessary for image sequences) ie. /%.05d.jpg", false, "", "string", _cmd );
    SwitchArg useDepth( "d", "depth", "Use an additional depth input", _cmd, false );
    _tracker = parseTrackerParas(_cmd, argc, argv);

    paras.deviceRGB = deviceIdArg.getValue();
    paras.deviceDepth = deviceIdArgDepth.getValue();
    paras.sequencePathRGB = seqPathArg.getValue();
    paras.sequencePathDepth = seqPathArgDepth.getValue();
    string expansionRGB = expansionArg.getValue();
    string expansionDepth = expansionArgDepth.getValue();
    string expansionRGB2 = "*.png";
    string expansionDepth2 = "*.png";
    paras.sequenceRGB = paras.sequencePathRGB + expansionRGB2;
    paras.sequencedepth = paras.sequencePathDepth + expansionDepth2;


    size_t foundExpansion = expansionRGB.find_first_of('.');

    if (foundExpansion != string::npos)
    {
      expansionRGB.erase(foundExpansion, 1);
    }

    foundExpansion = expansionDepth.find_first_of('.');

    if (foundExpansion != string::npos)
    {
      expansionDepth.erase(foundExpansion, 1);
    }

    paras.expansionRGB = expansionRGB;
    paras.expansionDepth = expansionDepth;

    paras.outputFilePath = outputPathArg.getValue();
    paras.imgExportPath = imgExportPathArg.getValue();
    paras.showOutput = !noShowOutputSw.getValue();
    paras.paused = pausedSw.getValue();
    paras.repeat = repeatSw.getValue();
    paras.startFrame = startFrameArg.getValue();

    stringstream initBbSs(initBbArg.getValue());

    double initBb[4];

    for (int i = 0; i < 4; ++i)
    {
      string singleValueStr;
      getline(initBbSs, singleValueStr, ',');
      initBb[i] = static_cast<double>(stod(singleValueStr.c_str()));
    }

    paras.initBb = Rect_<double>(initBb[0], initBb[1], initBb[2], initBb[3]);
    paras.useDepth = useDepth.getValue();

    if (_debug != 0)
    {
      _debug->init(paras.outputFilePath + "_debug");
    }

    paras.isMockSequenceRGB = dummySequenceSw.getValue();
    paras.isMockSequenceDepth = (dummySequenceSwDepth.getValue() || !useDepth.getValue());
  }
  catch (ArgException &argException)
  {
    cerr << "Command Line Argument Exception: " << argException.what() << endl;
    exit(-1);
  }
  // TODO: properly check every argument and throw exceptions accordingly
  catch (...)
  {
    cerr << "Command Line Argument Exception!" << endl;
    exit(-1);
  }

  return paras;
}

bool TrackerRun::start(int argc, const char** argv)
{
  _paras = parseCmdArgs(argc, argv);

  while (true)
  {
    if ( !init() )
    {
      return false;
    }
    if ( !run() )
    {
      return false;
    }
    if (!_paras.repeat || _exit)
    {
      break;
    }

    _boundingBox = _paras.initBb;
    _isTrackerInitialzed = false;
  }

  return true;
}

bool TrackerRun::init()
{
  ImgAcqParas imgAcqParas;
  imgAcqParas.device = _paras.deviceRGB;
  imgAcqParas.expansionStr = _paras.expansionRGB;
  imgAcqParas.isMock = _paras.isMockSequenceRGB;
  imgAcqParas.sequencePath = _paras.sequencePathRGB;
  _cap[ 0 ].open(imgAcqParas);

  if (!_cap[ 0 ].isOpened())
  {
    cerr << "Could not open device/sequence/video!" << endl;
    //exit(-1);
  }

  if( _paras.useDepth )
  {
    imgAcqParas.device = _paras.deviceDepth;
    imgAcqParas.expansionStr = _paras.expansionDepth;
    imgAcqParas.isMock = _paras.isMockSequenceDepth;
    imgAcqParas.sequencePath = _paras.sequencePathDepth;
    _cap[ 1 ].open( imgAcqParas );

    if( !_cap[ 1 ].isOpened() )
    {
      cerr << "Could not open device/sequence/video!" << endl;
      //exit( -1 );
    }

    //_cap[ 1 ].set( CV_CAP_PROP_CONVERT_RGB, 0 );
    //_cap[ 1 ].set( CV_CAP_PROP_FORMAT, CV_16UC1 );
  }

  int startIdx = _paras.startFrame - 1;

  // HACKFIX:
  //_cap.set(CV_CAP_PROP_POS_FRAMES, startIdx);
  // OpenCV's _cap.set in combination with image sequences is
  // currently bugged on Linux
  // TODO: review when OpenCV 3.0 is stable
  std::array< cv::Mat, 2 > temp;

  for (int i = 0; i < startIdx; ++i)
  {
    _cap[ 0 ] >> temp[ 0 ];

    if( _paras.useDepth )
    {
      _cap[ 1 ] >> temp[ 1 ];
    }
  }
    // HACKFIX END

  if (_paras.showOutput)
  {
    namedWindow(_windowTitle.c_str());
  }

  if (!_paras.outputFilePath.empty())
  {
    _resultsFile.open(_paras.outputFilePath.c_str());

    if (!_resultsFile.is_open())
    {
      std::cerr << "Error: Unable to create results file: "
        << _paras.outputFilePath.c_str()
        << std::endl;

      return false;
    }

    _resultsFile.precision(std::numeric_limits<double>::digits10 - 4);
  }

  if (!_paras.outputFilePath.empty())
  {
	 size_t posLast= _paras.outputFilePath.find_last_of('/');

	 string nameTimeFile=_paras.outputFilePath.substr(0,posLast);
	 nameTimeFile+="/modulesTimeFrames.txt";
    _resultsFileTime.open(nameTimeFile.c_str());

    if (!_resultsFileTime.is_open())
    {
      std::cerr << "Error: Unable to create results file: "
        << _paras.outputFilePath.c_str()
        << std::endl;

      return false;
    }

    _resultsFileTime.precision(std::numeric_limits<double>::digits10 - 16);
  }

  if (_paras.initBb.width > 0 || _paras.initBb.height > 0)
  {
    _boundingBox = _paras.initBb;
    _hasInitBox = true;
  }

  _isPaused = _paras.paused;
  _frameIdx = 0;
  // std::string sequenceRGB = _paras.sequencePathRGB+_paras.expansionRGB;

  // std::string sequencedepth = _paras.sequencePathDepth+_paras.expansionDepth;
  // std::string sequenceRGB = "ValidationSet/bear_front/img/*.png";
  // std::string sequencedepth = "ValidationSet/bear_front/depth2/*.png";
  // std::string sequenceRGB = "ValidationSet/zcup_move_1/img/*.png";
  // std::string sequencedepth = "ValidationSet/zcup_move_1/depth/*.png";
  // std::vector<cv::String> image_rgb;
  // std::vector<cv::String> image_depth;
  // cout<<sequenceRGB<<endl;
  cv::glob(_paras.sequenceRGB, image_rgb);
  // cout<<1<<endl;
  // cout<<sequenceRGB<<endl;
  cv::glob(_paras.sequencedepth, image_depth);
  return true;
}

bool TrackerRun::run()
{
  /*std::cout << "Switch pause with 'p'" << std::endl;
  std::cout << "Step frame with 'c'" << std::endl;
  std::cout << "Select new target with 'r'" << std::endl;
  if( _paras.useDepth )
  {
    std::cout << "Toggle view depth 'd'" << std::endl;
  }
  std::cout << "Update current tracker model at new location  't'" << std::endl;
  std::cout << "Quit with 'ESC'" << std::endl;*/

  while (true)
  {
    bool success = update();
    if (!success)
    {
      break;
    }
  }

  _cap[ 0 ].release();

  if( _paras.useDepth )
  {
    _cap[ 1 ].release();
  }

  return true;
}

bool TrackerRun::update()
{
  int64 tStart = 0;
  int64 tDuration = 0;


  if (!_isPaused || _frameIdx == 0 || _isStep)
  {

    // _cap[ 0 ].read(_image[ 0 ]);
    // cout<<1<<endl;
    // cout<<image_rgb[0]<<endl;
    _image[0] = cv::imread(image_rgb[_frameIdx]);
    if(_image[0].channels()>1)
    {
      cvtColor(_image[0], _image[0], COLOR_BGR2GRAY);
      cvtColor(_image[0], _image[0], COLOR_GRAY2BGR);
    }
    // _image[1] = cv::imread(image_depth[_frameIdx], -1);

    if( _paras.useDepth )
    {
      // _cap[ 1 ].read(_image[ 1 ]);
      _image[1] = cv::imread(image_depth[_frameIdx], -1);
      // cout<<image_rgb[_frameIdx]<<endl;
      // cout<<image_depth[_frameIdx]<<endl;
      // cout<<"image:"<<" "<<_image[1].size()<<endl; 
      // if (_image[1].channels()>1)
      // {
      //   cvtColor(_image[1], _image[1], COLOR_BGR2GRAY);
      // }
    }

    if (_image[ 0 ].empty())
    {
      return false;
    }

    if( _paras.useDepth && _image[ 1 ].empty() )
    {
      return false;
    }

    ++_frameIdx;
  }

  if (!_isTrackerInitialzed)
  {
    if (!_hasInitBox)
    {
      Rect box;

      if (!InitBoxSelector::selectBox(_image[ this->_imageIndex ], box))
      {
        return false;
      }

      _boundingBox = Rect_<double>(static_cast<double>(box.x),
        static_cast<double>(box.y),
        static_cast<double>(box.width),
        static_cast<double>(box.height));

        _hasInitBox = true;
    }

    tStart = getTickCount();

		std::array< cv::Mat, 2 > resized;
		cv::resize( _image[ 0 ], resized[ 0 ], cv::Size_< double >( _image[ 0 ].cols / SCALE, _image[ 0 ].rows / SCALE ), 0, 0, cv::INTER_LINEAR);
		cv::resize( _image[ 1 ], resized[ 1 ], cv::Size_< double >( _image[ 1 ].cols / SCALE, _image[ 1 ].rows / SCALE ), 0, 0, cv::INTER_NEAREST);
		cv::Rect_< double > r(
			_boundingBox.x / SCALE, _boundingBox.y / SCALE,
			_boundingBox.width / SCALE, _boundingBox.height / SCALE
		);

    _targetOnFrame = _tracker->reinit(resized, r);
    tDuration = getTickCount() - tStart;

    if( _targetOnFrame )
    {
#ifdef __linux
      std::cout << "   " << _boundingBox.x
      << "   " << _boundingBox.y
      << "   " << _boundingBox.x + _boundingBox.width
      << "   " << _boundingBox.y + _boundingBox.height
      << "   " << 0
      << std::endl;
#endif
    }

    if (_targetOnFrame)
    {
      _isTrackerInitialzed = true;
    }
  }
  else if (_isTrackerInitialzed && (!_isPaused || _isStep))
  {
    _isStep = false;
    tStart = getTickCount();

		std::array< cv::Mat, 2 > resized;
  	cv::resize( _image[ 0 ], resized[ 0 ], cv::Size_< double >( _image[ 0 ].cols / SCALE, _image[ 0 ].rows / SCALE ), 0, 0, cv::INTER_LINEAR);
  	cv::resize( _image[ 1 ], resized[ 1 ], cv::Size_< double >( _image[ 1 ].cols / SCALE, _image[ 1 ].rows / SCALE ), 0, 0, cv::INTER_NEAREST);

		cv::Rect_< double > r(
			_boundingBox.x / SCALE, _boundingBox.y / SCALE,
			_boundingBox.width / SCALE, _boundingBox.height / SCALE
		);

	
	//Tracker Running HERE
	 std::vector<int64> singleFrameTiming(8);
	 //little value
	 for(int i=0; i<singleFrameTiming.size()-1; i++)
	 {
		singleFrameTiming[i]=i+1;	
	 }

    _targetOnFrame = _tracker->update(resized, r,singleFrameTiming);
	 printResultsTiming(singleFrameTiming);

		_boundingBox.x = r.x * SCALE;
		_boundingBox.y = r.y * SCALE;
		_boundingBox.width = r.width * SCALE;
		_boundingBox.height = r.height * SCALE;

    _boundingBox = rectRound( _boundingBox );

#ifdef __linux
    if( _targetOnFrame )
    {
      std::cout << "   " << _boundingBox.x
        << "   " << _boundingBox.y
        << "   " << _boundingBox.x + _boundingBox.width
        << "   " << _boundingBox.y + _boundingBox.height
        << "   " << 0
        << std::endl;
    }
    else
    {
      std::cout << "   NaN   NaN   NaN   NaN   " << 1 << std::endl;
    }
#endif
    tDuration = getTickCount() - tStart;
  }

  this->frameTime.push_back( static_cast< double >( tDuration ) / static_cast< double >( getTickFrequency() ) );
  double fps = static_cast<double>(getTickFrequency() / tDuration);
  printResults(_boundingBox, _targetOnFrame, _targetOnFrame);

  if (_paras.showOutput)
  {
    Scalar colour = _targetOnFrame ? Scalar(0, 255, 0) : Scalar(0, 0, 255);
    Mat hudImage;
    _image[ _imageIndex ].copyTo(hudImage);
    rectangle(hudImage, _boundingBox, colour, 2);
    Point_<double> center;
    center.x = _boundingBox.x + _boundingBox.width / 2;
    center.y = _boundingBox.y + _boundingBox.height / 2;
    circle(hudImage, center, 3, colour, 2);

    stringstream ss;
    ss << "FPS: " << fps;
    putText(hudImage, ss.str(), Point(20, 20), FONT_HERSHEY_TRIPLEX, 0.5, Scalar(255, 0, 0));

    ss.str("");
    ss.clear();
    ss << "#" << _frameIdx;
    putText(hudImage, ss.str(), Point(hudImage.cols - 60, 20), FONT_HERSHEY_TRIPLEX, 0.5, Scalar(255, 0, 0));

    if (_debug != 0)
    {
      _debug->printOnImage(hudImage);
    }

    if (!_targetOnFrame)
    {
      cv::Point_<double> tl = _boundingBox.tl();
      cv::Point_<double> br = _boundingBox.br();

      line(hudImage, tl, br, colour);
      line(hudImage, cv::Point_<double>(tl.x, br.y),
        cv::Point_<double>(br.x, tl.y), colour);
    }

    imshow(_windowTitle.c_str(), hudImage);

    if (!_paras.imgExportPath.empty())
    {
      stringstream ssi;
      ssi << setfill('0') << setw(5) << _frameIdx << ".png";
      std::string imgPath = _paras.imgExportPath + ssi.str();

      try
      {
        imwrite(imgPath, hudImage);
      }
      catch (runtime_error& runtimeError)
      {
        cerr << "Could not write output images: " << runtimeError.what() << endl;
      }
    }

    char c = (char)waitKey(10);

    if (c == 27)
    {
      _exit = true;
      return false;
    }

    switch (c)
    {
    case 'p':
      _isPaused = !_isPaused;
      break;
    case 'c':
      _isStep = true;
      break;
    case 'r':
      _hasInitBox = false;
      _isTrackerInitialzed = false;
      break;
    case 't':
      _updateAtPos = true;
      break;
    case 'd':
      if( _paras.useDepth )
      {
        _imageIndex = ( _imageIndex + 1 ) % 2;
      }
      else
      {
        _imageIndex = 0;
      }
      break;
    default:
      break;
    }
  }
  waitKey( 1 );

  return true;
}

void TrackerRun::printResults(const cv::Rect_<double>& boundingBox, bool isConfident, bool isTracked)
{
  if (_resultsFile.is_open())
  {
    if (boundingBox.width > 0 && boundingBox.height > 0 && isConfident && isTracked)
    {
      _resultsFile << boundingBox.x << ","
        << boundingBox.y << ","
        << boundingBox.width +boundingBox.x<< ","
        << boundingBox.height +boundingBox.y<< ","
        << 0 << std::endl;
    }
    else
    {
      _resultsFile << "NaN, NaN, NaN, NaN, " << 1 << std::endl;
    }

    if (_debug != 0)
    {
      _debug->printToFile();
    }
  }
}

void TrackerRun::printResultsTiming(const std::vector<int64> &singleFrameTiming)
{
  if (_resultsFileTime.is_open())
  {
	double fticks=static_cast< double >( getTickFrequency() );

    for(int i=0; i<singleFrameTiming.size(); i++)
    {
		double elapsedTime=0;
		
		if(singleFrameTiming[i]!=0)
			elapsedTime = static_cast< double >( singleFrameTiming[i] ) / fticks;	
		_resultsFileTime << elapsedTime << " ";
	}
	_resultsFileTime << endl;
  }
}

void TrackerRun::setTrackerDebug(TrackerDebug* debug)
{
  _debug = debug;
}
