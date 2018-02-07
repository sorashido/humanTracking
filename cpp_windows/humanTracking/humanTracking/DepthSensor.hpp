#pragma once

#include <opencv2\core\core.hpp>
#include "pxcsensemanager.h"

const int DEPTH_HEIGHT = 240;
const int DEPTH_WIDTH = 320;

class DepthSensor {
public:
	DepthSensor(const wchar_t* filename);// { init(filename); }	
	~DepthSensor();
	void getFrame(int frame, cv::Mat* depthMat, PXCPoint3DF32 *vertices);
	int nframes;
	void frameRelease();

	void cameraToWorld(PXCPoint3DF32 *camera, PXCPoint3DF32 *world);
	void cameraToWorldPoint(PXCPoint3DF32 * camera, PXCPoint3DF32 * world);
private:
	void ConvertPXCImageToOpenCVMat(PXCImage *inImg, cv::Mat *outImg);
};