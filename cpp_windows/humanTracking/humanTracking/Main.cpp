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

typedef struct {
	double x;
	double y;
	double d;
	double width;
	double height;
	int frame;
	int id;
}personInf;
std::vector<personInf> people;//
std::vector<std::vector<personInf>> track_data;

int main(){
	DepthSensor sensor(L"D:\\track_data\\No6_out2017-11-03 6-08-56.rssdk");
	//DepthSensor sensor(L"F:\\深度センサ記録\\20171103_Area A休日\\No6_out2017-11-03 6-08-56.rssdk");

	Labeling label;
	cv::Mat depthMat(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);
	//Mat trackMat = Mat::zeros(cv::Size(640, 480), CV_8UC3);

	//std::vector<Intel::RealSense::Point3DF32> camera, world;
	//camera.resize(320 * 240);
	//world.resize(320 * 240);
	static Intel::RealSense::Point3DF32 cameraPoint, worldPoint;

	// window setting
	const static std::string WINDOWNAME = "Depth";
	cv::namedWindow(WINDOWNAME);
	cv::setMouseCallback(WINDOWNAME, onMouse);

	int view_mode = 0;
	
	// Streaming loop
	for (int i = 0; i < (int)sensor.nframes; i += 1) {
		sensor.getFrame(i, &depthMat);

		label.labeling(depthMat);

		//
		for (auto r : label.results) {
			cameraPoint.x = r.x;
			cameraPoint.y = r.y;
			cameraPoint.z = r.d;
			sensor.cameraToWorldPoint(&cameraPoint, &worldPoint);
		}

		// drawing
		cv::Mat depthTmp, paintMat;
		depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);
		cv::cvtColor(depthTmp, paintMat, CV_GRAY2BGR);
		const static int rate = 2.0;
		cv::resize(paintMat, paintMat, cv::Size(), rate, rate);
		
		// draw frame
		char str[200];
		sprintf_s(str, "%4d", i);
		cv::putText(paintMat, str, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(244, 67, 57), 2, CV_AA);

		//sprintf_s(str, "%4d, %4d, %4d", (int)world[m_y/rate * 320 + m_x/rate].x, (int)world[m_y/rate * 320 + m_x/rate].y, (int)world[m_y/rate * 320 + m_x/rate].z);
		//cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(7, 193, 255), 2, CV_AA);

		switch (view_mode) {
		case 0://non_draw
			break;
		case 1://debug mode
			for (auto r : label.results) {
				cv::rectangle(paintMat, Point(r.x*rate - r.width*rate / 2, r.y*rate - r.height*rate / 2), Point(r.x*rate + r.width*rate / 2, r.y*rate + r.height*rate / 2), Scalar(136, 150, 0), 2);
				sprintf_s(str, "%4d", (int)r.d);
				cv::putText(paintMat, str, cv::Point(r.x*rate, r.y*rate), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(54, 67, 244), 2, CV_AA);
				//cv::circle(paintMat, cv::Point(worldPoint.x * rate, -worldPoint.z * 480 / 4000 + 650), 5, cv::Scalar(54, 67, 244));
			}
			break;
		case 9:
			// stop
			i -= 1;
			break;
		default:
			break;
		}

		cv::imshow(WINDOWNAME, paintMat);
		int key = cv::waitKey(10);
		if (key >= '0' && key <= '9') view_mode = key - '0';
		else if (key == 'q') break;

		sensor.frameRelease();
	}
	return 0;
}

