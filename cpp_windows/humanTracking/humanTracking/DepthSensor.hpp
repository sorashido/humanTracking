#pragma once

#include <opencv2\core\core.hpp>
#include "pxcsensemanager.h"

const int DEPTH_HEIGHT = 240;
const int DEPTH_WIDTH = 320;

typedef struct {
	double x;
	double y;
	double z;
}Point3D;

class DepthSensor {
public:
	DepthSensor(const wchar_t* filename);// { init(filename); }	
	~DepthSensor();
	//void getFrame(int frame, cv::Mat* depthMat, PXCPoint3DF32 *vertices);
	void getFrame(int frame, cv::Mat* depthMat);
	int nframes;
	void frameRelease();

	//void cameraToWorld(PXCPoint3DF32 *camera, PXCPoint3DF32 *world);
	void cameraToWorldPoint(Point3D *camera, Point3D *world);
	//void cameraToWorldPoint(PXCPoint3DF32 * camera, PXCPoint3DF32 * world);
private:
	void ConvertPXCImageToOpenCVMat(PXCImage *inImg, cv::Mat *outImg);
};