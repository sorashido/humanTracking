#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "pxcsensemanager.h"
#include "Labeling.hpp"
#include "DepthSensor.hpp"

using namespace std;
using namespace cv;

//#define DEBUG_LOG

int m_x, m_y, m_event;

void onMouse(int event, int x, int y, int flags, void *param = NULL)
{
	m_event = event;
	m_x = x;
	m_y = y;
}

void ConvertPXCImageToOpenCVMat(PXCImage *inImg, Mat *outImg) {
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

// Convert PXCImage to opencv Mat (YUY2 format)
void RSSDKConvert(const wchar_t* filename)
{
	PXCSenseManager *sm = PXCSenseManager::CreateInstance();

#ifdef DEBUG_LOG
	cout << "enter RSSDKConvert" << endl;
#endif

	sm->QueryCaptureManager()->SetFileName(filename, false);
	sm->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 0, 0);
	sm->Init();

#ifdef DEBUG_LOG
	cout << "enter Init" << filename << endl;
#endif

	PXCImage *depthIm;
	PXCImage::ImageData depth_data;
	PXCImage::ImageInfo depth_information;

	//Mat paintMat = Mat(480, 640, CV_8UC1);

	// Set realtime=true and pause=false
	sm->QueryCaptureManager()->SetRealtime(false);
	sm->QueryCaptureManager()->SetPause(true);

	int nframes = sm->QueryCaptureManager()->QueryNumberOfFrames();

#ifdef DEBUG_LOG
	cout << "nframes:\t" << nframes << endl;
#endif

	const static std::string WINDOWNAME = "Depth";	//
	cv::namedWindow(WINDOWNAME);				//	
	cv::setMouseCallback(WINDOWNAME, onMouse);	//

	//cv::FileStorage fs("test.xml", cv::FileStorage::WRITE);
	//if (!fs.isOpened()) {
	//	std::cout << "File can not be opened." << std::endl;
	//}
	Labeling label;		//ラベリング

	// Streaming loop
	cv::Mat depthMat(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);				//depthの取得データ

	for (int i = 0; i < nframes; i += 1) {

		// Set to work on every 3rd frame of data
		sm->QueryCaptureManager()->SetFrameByIndex(i);
		sm->FlushFrame();

		// Ready for the frame to be ready
		pxcStatus sts = sm->AcquireFrame(true);
		if (sts < PXC_STATUS_NO_ERROR) break;

		// Retrieve the sample and work on it. The image is in sample->color.
		PXCCapture::Sample* sample = sm->QuerySample();
		ConvertPXCImageToOpenCVMat(sample->depth, &depthMat);
		cv::resize(depthMat, depthMat, cv::Size(), 2.0, 2.0);

		// tracking
		label.labeling(depthMat, 1);
		
		// draw mat
		Mat depthTmp, paintMat;
		depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);
		cv::cvtColor(depthTmp, paintMat, CV_GRAY2BGR);

		char str[64];
		sprintf_s(str, "%4d", depthMat.at<short>(m_y, m_x));
		cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(255, 0, 0), 2, CV_AA);

		for (int y = 0; y < DEPTH_HEIGHT; y++) {
			for (int x = 0; x < DEPTH_WIDTH; x++) {
				int num = label.table.at<short>(y, x);
				if (num != 0 && num != INIT && label.isId.find(num) != label.isId.end()) {
					//std::cout << num << ",";
					paintMat.at<cv::Vec3b>(y, x)[0] = 255; //255 * (num % 2);
					paintMat.at<cv::Vec3b>(y, x)[1] = 0; //(255 / 4)*(num % 4);
					paintMat.at<cv::Vec3b>(y, x)[2] = 0; //(255 / 2)*(num % 3);
				}
			}
		}

		for (auto r : label.results) {
			cv::rectangle(paintMat, Point(r.x, r.y), Point(r.x + r.width, r.y + r.height), Scalar(0, 255, 0), 2);
		}

		cv::imshow(WINDOWNAME, paintMat);
		int key = cv::waitKey(1);
		if (key == 27)
			break;

		// Resume processing the next frame
		sm->ReleaseFrame();
	}

	//fs.release();
	sm->Release();
}

int main()
{
	RSSDKConvert(L"C:\\Users\\shiba\\Downloads\\No5_out2017-11-14 5-40-31.rssdk");
	//system("pause");
	return 0;
}