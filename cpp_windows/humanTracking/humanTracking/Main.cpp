#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "pxcsensemanager.h"
#include "Labeling.hpp"
#include "DepthSensor.hpp"

using namespace std;
using namespace cv;

int m_x = 0, m_y = 0, m_event = 0;
void onMouse(int event, int x, int y, int flags, void *param = NULL) {
	m_event = event;
	m_x = x;
	m_y = y;
}

int main(){
	DepthSensor sensor(L"F:\\深度センサ記録\\20171103_Area A休日\\No6_out2017-11-03 6-08-56.rssdk");
	Labeling label;
	cv::Mat depthMat(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);

	std::vector<Intel::RealSense::Point3DF32> vertices;
	vertices.resize(320 * 240);

	// window setting
	const static std::string WINDOWNAME = "Depth";
	cv::namedWindow(WINDOWNAME);
	cv::setMouseCallback(WINDOWNAME, onMouse);

	int view_mode = 0; //

	// Streaming loop
	// No.1
	// No.6 nframes = 1953147, init = 1000, best = 4500
	for (int i = 0; i < (int)sensor.nframes; i += 1) {
		sensor.getFrame(i, &depthMat, &vertices[0]);

		//switch mode
		// tracking
		label.labeling(depthMat);

		// perspective


		// draw mat
		cv::Mat depthTmp, paintMat;
		depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);
		cv::cvtColor(depthTmp, paintMat, CV_GRAY2BGR);

		// draw frame
		char str[64];
		sprintf_s(str, "%4d", i);
		cv::putText(paintMat, str, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(244, 67, 57), 2, CV_AA);

		// mouse depth
		//sprintf_s(str, "%4d", depthMat.at<short>(m_y, m_x));
		//cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(7, 193, 255), 2, CV_AA);
		sprintf_s(str, "%4d, %4d, %4d", (int)vertices[m_y/2*320+m_x/2].x, (int)vertices[m_y / 2 * 320 + m_x/2].y, (int)vertices[m_y / 2 * 320 + m_x/2].z);
		cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(7, 193, 255), 2, CV_AA);

		switch (view_mode) {
		case 1:
			// draw label result
			for (auto r : label.results) {
				cv::rectangle(paintMat, Point(r.x - r.width / 2, r.y - r.height / 2), Point(r.x + r.width / 2, r.y + r.height / 2), Scalar(136, 150, 0), 2);
				sprintf_s(str, "%4d", (int)r.d);// , (int)r.size);
				cv::putText(paintMat, str, cv::Point(r.x, r.y), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(54, 67, 244), 2, CV_AA);
			}
			break;
		case 2:
			// stop
			i -= 1;
			break;
		default:
			break;
		}

		cv::imshow(WINDOWNAME, paintMat);
		int key = cv::waitKey(1);
		if (key >= '0' && key <= '9') view_mode = key - '0';
		else if (key == 'q') break;

		sensor.frameRelease();
	}
	return 0;
}