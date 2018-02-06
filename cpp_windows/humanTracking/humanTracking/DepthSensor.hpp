#pragma once

#include <opencv2\core\core.hpp>
#include "pxcsensemanager.h"

const int DEPTH_HEIGHT = 480;
const int DEPTH_WIDTH = 640;

class DepthSensor {
public:
	DepthSensor(const wchar_t* filename);// { init(filename); }	
	~DepthSensor();
	void getFrame(int frame, cv::Mat* depthMat, PXCPoint3DF32 *vertices);
	//void getWorld(int frame, std::vector<Intel::RealSense::PointF32>& wcords);
	int nframes;
	void frameRelease();
private:
	void ConvertPXCImageToOpenCVMat(PXCImage *inImg, cv::Mat *outImg);
};