#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/tracking.hpp>
#include <memory>
#include "pxcsensemanager.h"
#include "Labeling.hpp"
#include "DepthSensor.hpp"
#include "samples_utility.hpp"

int m_x, m_y, m_event;
void onMouse(int event, int x, int y, int flags, void *param = NULL){
	m_event = event;
	m_x = x;
	m_y = y;
}

using namespace std;
using namespace cv;

int main(){
	//std::unique_ptr<DepthSensor> sensor(new DepthSensor(L"C:\\Users\\itolab\\Downloads\\No6_out2017-11-07 5-44-46.rssdk"));
	DepthSensor sensor(L"C:\\Users\\itolab\\Downloads\\No6_out2017-11-07 5-44-46.rssdk");
	Labeling label;
	cv::Mat depthMat(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);

	// window setting
	const static std::string WINDOWNAME = "Depth";
	cv::namedWindow(WINDOWNAME);
	cv::setMouseCallback(WINDOWNAME, onMouse);

	VideoCapture cap("videos/sample.mp4"); //Macの場合

	cv::String trackingAlg = "MIL";
	MultiTracker trackers;
	vector<Rect2d> objects;

	sensor.getFrame(4980, &depthMat);
	cv::Mat depthTmp, paintMat;
	depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);

	vector<Rect> ROIs;
	selectROIs("tracker", depthTmp, ROIs);
	if (ROIs.size() < 1)return 0;
	std::vector<Ptr<Tracker>> algorithms;
	for (size_t i = 0; i < ROIs.size(); i++) {
		algorithms.push_back(createTrackerByName(trackingAlg));
		objects.push_back(ROIs[i]);
	}
	trackers.add(algorithms, depthTmp, objects);

	// Streaming loop
	// No.6 nframes = 1953147, init = 1000, best = 4500
	for (int i = 4980; i < sensor.nframes; i += 1) {
		sensor.getFrame(i, &depthMat);

		// draw mat
		//cv::Mat depthTmp, paintMat;
		depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);

		// tracking
		//label.labeling(depthMat);

		trackers.update(depthTmp);

		// draw mat
		//cv::Mat depthTmp, paintMat;
		//depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);
		cv::cvtColor(depthTmp, paintMat, CV_GRAY2BGR);
		for (unsigned i = 0; i < trackers.getObjects().size(); i++)
			rectangle(paintMat, trackers.getObjects()[i], Scalar(255, 0, 0), 2, 1);

		//imshow("tracker", depthMat);

		// perspective
		

		// draw mat
		//cv::Mat depthTmp, paintMat;
		//depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);
		//cv::cvtColor(depthTmp, paintMat, CV_GRAY2BGR);

		// draw frame
		char str[64];
		sprintf_s(str, "%4d", i);
		cv::putText(paintMat, str, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(244, 67, 57), 2, CV_AA);

		// draw label result
		//for (auto r : label.results) {
		//	cv::rectangle(paintMat, Point(r.x - +r.width / 2, r.y - r.height / 2), Point(r.x + r.width / 2, r.y + r.height / 2), Scalar(136, 150, 0), 2);
		//	sprintf_s(str, "%4d", (int)r.d);// , (int)r.size);
		//	cv::putText(paintMat, str, cv::Point(r.x, r.y), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(54, 67, 244), 2, CV_AA);
		//}

		//

		// depth
		//sprintf_s(str, "%4d", depthMat.at<short>(m_y, m_x));
		//cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(7, 193, 255), 2, CV_AA);

		cv::imshow(WINDOWNAME, paintMat);
		int key = cv::waitKey(10);
		if (key == 'q')
			break;

		sensor.frameRelease();
	}
	return 0;
}