#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/bgsegm.hpp>
#include <random>
#include "pxcsensemanager.h"
#include "Detecting.hpp"
#include "Tracking.hpp"
#include "DepthSensor.hpp"

using namespace std;
using namespace cv;

void drawFrame(int* frame);
int m_x = 0, m_y = 0, m_event = 0;
void onMouse(int event, int x, int y, int flags, void *param = NULL) {
	m_event = event;
	m_x = x;
	m_y = y;
}

cv::Mat depthMat(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);
std::vector<PXCPoint3DF32> vertices;

//std::vector<personInf> people;
std::vector<detection> detections;
std::vector<std::vector<detection>> track_data;

const static std::string WINDOWNAME = "Depth";

//#define LOG
#ifdef LOG
	ofstream myfile;
#endif

int main(){
//	DepthSensor sensor(L"D:\\深度センサ記録\\20171103_Area A休日\\No6_out2017-11-03 5-49-52.rssdk");
	DepthSensor sensor(L"D:\\深度センサ記録\\20171103_Area A休日\\No6_out2017-11-03 6-08-56.rssdk");

	Detect detector;
	Track tracker;

	// window setting
	cv::namedWindow(WINDOWNAME);
	cv::setMouseCallback(WINDOWNAME, onMouse);

#ifdef LOG
	myfile.open("../../../log/20171103_Area_A/no1.csv");
	myfile << "frame" << "," << "id" << "," << "wx" << "," << "wy" << "," << "wz" << "\n";
#endif

	vertices.resize(DEPTH_HEIGHT*DEPTH_WIDTH);
	
	cv::Ptr<cv::BackgroundSubtractor> bgfs = cv::bgsegm::createBackgroundSubtractorGSOC();

	// Streaming loop
	bool stop = false;
	for (int i = 0;; i += 1) {
		sensor.getFrame(i, &depthMat, &vertices[0]);

		//cv::Mat foreGroundMask, segm;
		//bgfs->apply(depthMat, foreGroundMask);
		//depthMat.convertTo(segm, 0, 0.5);
		//cv::add(depthMat, cv::Scalar(100, 100, 100), segm, foreGroundMask);
		//cv::imshow("out", segm);

		// detections
		detections.clear();
		detector.detectPeople(&sensor, i, depthMat, &vertices[0], &detections);

		// tracking
		tracker.trackPeople(&detections, &track_data);

		// drawing
		drawFrame(&i);

		int key = cv::waitKey(10);
		if (key == 'q') break;
		else if (key == 32) stop = !stop;

		if (stop)i -= 1;
		sensor.frameRelease();
	}
#ifdef LOG
	myfile.close();
#endif
	return 0;
}

void drawFrame(int* frame) {
	// drawing
	Mat trackMat = Mat::zeros(cv::Size(640, 480), CV_8UC3);
	cv::Mat depthTmp, paintMat;
	depthMat.convertTo(depthTmp, CV_8U, 255.0f / 8000.0f, 0);
	cv::cvtColor(depthTmp, paintMat, CV_GRAY2BGR);
	const static int rate = 2.0;
	cv::resize(paintMat, paintMat, cv::Size(), rate, rate);

	char str[200];
	sprintf_s(str, "%4d", *frame);
	cv::putText(paintMat, str, cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(244, 67, 57), 2, CV_AA);

	sprintf_s(str, "%4d, %4d, %4d", (int)vertices[m_y / rate * 320 + m_x / rate].x, (int)vertices[m_y / rate * 320 + m_x / rate].y, (int)vertices[m_y / rate * 320 + m_x / rate].z);
	cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(7, 193, 255), 2, CV_AA);

	const static cv::Scalar color[10] = { cv::Scalar(244, 67, 54), cv::Scalar(63, 81, 181), cv::Scalar(205, 220, 57),
		cv::Scalar(255, 152, 0), cv::Scalar(121, 85, 72), cv::Scalar(233, 30, 99),
		cv::Scalar(156, 39, 176), cv::Scalar(33, 150, 243), cv::Scalar(255, 235, 59), cv::Scalar(255, 255, 255) };

	for (auto t : track_data) {
		// now data
		detection now_p = t.back();
		if (now_p.frame == *frame) {
			cv::rectangle(paintMat, Point(now_p.sx*rate - now_p.width*rate / 2, now_p.sy*rate - now_p.height*rate / 2), Point(now_p.sx*rate + now_p.width*rate / 2, now_p.sy*rate + now_p.height*rate / 2), Scalar(136, 150, 0), 2);
			sprintf_s(str, "%4d", (int)now_p.id);
			cv::putText(paintMat, str, cv::Point(now_p.sx*rate, now_p.sy*rate), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(54, 67, 244), 2, CV_AA);
#ifdef LOG
			myfile << now_p.frame << "," << (int)now_p.id << "," << now_p.wx << "," << now_p.wy << "," << now_p.wz << "\n";
#endif
		}

		// track data
		for (auto r : t) {
			if (*frame - r.frame < 40) {
				cv::circle(trackMat, cv::Point(r.wx * rate, -r.wz * 480 / 4000 + 650), 5, color[now_p.id % 10]);
			}
		}
	}

	cv::imshow("track", trackMat);
	cv::imshow(WINDOWNAME, paintMat);
}
