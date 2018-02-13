#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "DepthSensor.hpp"
#include "pxcsensemanager.h"

static PXCSenseManager *sm = PXCSenseManager::CreateInstance();

DepthSensor::DepthSensor(const wchar_t * filename)
{
	sm->QueryCaptureManager()->SetFileName(filename, false);
	sm->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 0, 0);
	sm->Init();

	PXCImage *depthIm;
	PXCImage::ImageData depth_data;
	PXCImage::ImageInfo depth_information;

	sm->QueryCaptureManager()->SetRealtime(false);
	sm->QueryCaptureManager()->SetPause(true);

	int nframes = sm->QueryCaptureManager()->QueryNumberOfFrames();
}

DepthSensor::~DepthSensor() {
	sm->Release();
}

void DepthSensor::frameRelease() {
	// Resume processing the next frame
	sm->ReleaseFrame();
}

void DepthSensor::getFrame(int frame, cv::Mat* depthMat, PXCPoint3DF32* vertices) {
	// Set to work on every 3rd frame of data
	sm->QueryCaptureManager()->SetFrameByIndex(frame);
	sm->FlushFrame();

	// Ready for the frame to be ready
	pxcStatus sts = sm->AcquireFrame(true);
	if (sts < PXC_STATUS_NO_ERROR) return;

	// Retrieve the sample and work on it. The image is in sample->color.
	PXCCapture::Sample* sample = sm->QuerySample();

	//projection
	static PXCProjection *projection = sm->QueryCaptureManager()->QueryDevice()->CreateProjection();

	projection->QueryVertices(sample->depth, vertices);

	ConvertPXCImageToOpenCVMat(sample->depth, depthMat);
}

const static cv::Mat m_perspectiveMat = (cv::Mat_<double>(4, 4) << 1.00000000e+00, 0.00000000e+00, 0.00000000e+00, 0.00000000e+00,
													  0.00000000e+00, 7.07106781e-01, -7.07106781e-01, 0.00000000e+00,
													  0.00000000e+00, 7.07106781e-01, 7.07106781e-01, 0.00000000e+00,
													  0.00000000e+00, 2.47487373e+03, -2.47487373e+03, 1.00000000e+00);
static cv::Mat localMat(4, 1, CV_64FC1);
static cv::Mat worldMat(4, 1, CV_64FC1);
void DepthSensor::cameraToWorldPoint(Point3D *camera, Point3D *world) {
	localMat.at<double>(0, 0) = camera->x;
	localMat.at<double>(1, 0) = camera->y;
	localMat.at<double>(2, 0) = camera->z;
	localMat.at<double>(3, 0) = 1;

	worldMat = m_perspectiveMat * localMat;

	world->x = worldMat.at<double>(0, 0);
	world->y = worldMat.at<double>(1, 0);
	world->z = worldMat.at<double>(2, 0);
}

void DepthSensor::worldToCameraPoint(Point3D *world, Point3D *camera) {
	worldMat.at<double>(0, 0) = world->x;
	worldMat.at<double>(1, 0) = world->y;
	worldMat.at<double>(2, 0) = world->z;
	worldMat.at<double>(3, 0) = 1;

	localMat = m_perspectiveMat.inv() * worldMat;

	camera->x = localMat.at<double>(0, 0);
	camera->y = localMat.at<double>(1, 0);
	camera->z = localMat.at<double>(2, 0);
}

void DepthSensor::ConvertPXCImageToOpenCVMat(PXCImage *inImg, cv::Mat *outImg) {
	int cvDataType;
	int cvDataWidth;

	PXCImage::ImageData data;
	inImg->AcquireAccess(PXCImage::ACCESS_READ, &data);
	PXCImage::ImageInfo imgInfo = inImg->QueryInfo();

	switch (data.format) {
		/* STREAM_TYPE_COLOR */
	case PXCImage::PIXEL_FORMAT_YUY2: /* YUY2 image  */
#ifdef DEBUG_LOG
		cout << "PXCImage Format not supported:PIXEL_FORMAT_YUY2" << endl;
#endif
		throw(0); // Not implemented
	case PXCImage::PIXEL_FORMAT_NV12: /* NV12 image */

#ifdef DEBUG_LOG
		cout << "PXCImage Format not supported:PIXEL_FORMAT_NV12" << endl;
#endif
		throw(0); // Not implemented
	case PXCImage::PIXEL_FORMAT_RGB32: /* BGRA layout on a little-endian machine */
		cvDataType = CV_8UC4;
		cvDataWidth = 4;
		break;
	case PXCImage::PIXEL_FORMAT_RGB24: /* BGR layout on a little-endian machine */
		cvDataType = CV_8UC3;
		cvDataWidth = 3;
		break;
	case PXCImage::PIXEL_FORMAT_Y8:  /* 8-Bit Gray Image, or IR 8-bit */
		cvDataType = CV_8U;
		cvDataWidth = 1;
		break;

		/* STREAM_TYPE_DEPTH */
	case PXCImage::PIXEL_FORMAT_DEPTH: /* 16-bit unsigned integer with precision mm. */
	case PXCImage::PIXEL_FORMAT_DEPTH_RAW: /* 16-bit unsigned integer with device specific precision (call device->QueryDepthUnit()) */
		cvDataType = CV_16U;
		cvDataWidth = 2;
		break;
	case PXCImage::PIXEL_FORMAT_DEPTH_F32: /* 32-bit float-point with precision mm. */
		cvDataType = CV_32F;
		cvDataWidth = 4;
		break;

		/* STREAM_TYPE_IR */
	case PXCImage::PIXEL_FORMAT_Y16:          /* 16-Bit Gray Image */
		cvDataType = CV_16U;
		cvDataWidth = 2;
		break;
	case PXCImage::PIXEL_FORMAT_Y8_IR_RELATIVE:    /* Relative IR Image */
		cvDataType = CV_8U;
		cvDataWidth = 1;
		break;
	}

#ifdef DEBUG_LOG
	cout << "after switch" << endl;
#endif

	// suppose that no other planes
	if (data.planes[1] != NULL) throw(0); // not implemented
										  // suppose that no sub pixel padding needed
	if (data.pitches[0] % cvDataWidth != 0) throw(0); // not implemented

	outImg->create(imgInfo.height, data.pitches[0] / cvDataWidth, cvDataType);

	memcpy(outImg->data, data.planes[0], imgInfo.height*imgInfo.width*cvDataWidth * sizeof(pxcBYTE));

	inImg->ReleaseAccess(&data);
}
