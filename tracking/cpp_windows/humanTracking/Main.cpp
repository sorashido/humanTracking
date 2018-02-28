#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <random>
#include "pxcsensemanager.h"
#include "Detecting.hpp"
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
	DepthSensor sensor(L"D:\\track_data\\No6_out2017-11-03 6-08-56.rssdk");

	Detect detect;
	cv::Mat depthMat(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);

	std::vector<personInf> people;//1 frame
	std::vector<std::vector<personInf>> track_data;

	static Point3D cameraPoint, worldPoint;

	// window setting
	const static std::string WINDOWNAME = "Depth";
	cv::namedWindow(WINDOWNAME);
	cv::setMouseCallback(WINDOWNAME, onMouse);
	int view_mode = 0;

#ifdef LOG
	ofstream myfile;
	myfile.open("../../../data/sample.csv");
	myfile << "frame" << "," << "id" << "," << "wx" << "," << "wy" << "," << "wz" << "\n";
#endif

	std::vector<PXCPoint3DF32> vertices;
	vertices.resize(DEPTH_HEIGHT*DEPTH_WIDTH);
	
	// Streaming loop
	int people_num = 0;
	bool stop = false;
	for (int i = 9000; i < 200000; i += 1) {
		sensor.getFrame(i, &depthMat, &vertices[0]);

		people.clear();
		detect.detectPeople(&sensor, i, depthMat, &vertices[0], &people);

		// tracking
		Mat trackMat = Mat::zeros(cv::Size(640, 480), CV_8UC3);

		bool isadd = false;
		std::vector<personInf> tt;
		for (auto p : people) {
			for (auto t = track_data.begin(); t != track_data.end(); ++t) {
				personInf tmp = t->back();
				double rate = sqrt(p.wx*p.wx + p.wz*p.wz)/5;
				if (sqrt(abs(tmp.wx - p.wx)*abs(tmp.wx - p.wx) + abs(tmp.wz - p.wz)*abs(tmp.wz - p.wz)) < rate && (p.frame - tmp.frame) < 10) {
					p.id = t->back().id;
					t->push_back(p);
					isadd = true;
				}
			}

			if (!isadd) {
				std::vector<personInf> per;
				p.id = people_num;
				people_num += 1;
				per.push_back(p);
				track_data.push_back(per);
			}
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

		sprintf_s(str, "%4d, %4d, %4d", (int)vertices[m_y / rate * 320 + m_x / rate].x, (int)vertices[m_y / rate * 320 + m_x / rate].y, (int)vertices[m_y / rate * 320 + m_x / rate].z);
		cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(7, 193, 255), 2, CV_AA);

		const static cv::Scalar color[10] = {cv::Scalar(244, 67, 54), cv::Scalar(63, 81, 181), cv::Scalar(205, 220, 57),
								cv::Scalar(255, 152, 0), cv::Scalar(121, 85, 72), cv::Scalar(233, 30, 99),
								cv::Scalar(156, 39, 176), cv::Scalar(33, 150, 243), cv::Scalar(255, 235, 59), cv::Scalar(255, 255, 255)};

		for (auto t : track_data) {
			// draw now frame data
			personInf now_p = t.back();
			if (now_p.frame == i) {
				cv::rectangle(paintMat, Point(now_p.sx*rate - now_p.width*rate / 2, now_p.sy*rate - now_p.height*rate / 2), Point(now_p.sx*rate + now_p.width*rate / 2, now_p.sy*rate + now_p.height*rate / 2), Scalar(136, 150, 0), 2);
				sprintf_s(str, "%4d", (int)now_p.id);
				cv::putText(paintMat, str, cv::Point(now_p.sx*rate, now_p.sy*rate), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(54, 67, 244), 2, CV_AA);
#ifdef LOG
				myfile << now_p.frame << "," << (int)now_p.id << "," << now_p.wx << "," << now_p.wy << "," << now_p.wz << "\n";
#endif
			}

			// draw track data
			for (auto r : t) {
				if (i - r.frame < 40) {
					cv::circle(trackMat, cv::Point(r.wx * rate, -r.wz * 480 / 4000 + 650), 5, color[now_p.id%10]);
				}
			}
		}
		cv::imshow("track", trackMat);
		cv::imshow(WINDOWNAME, paintMat);
		int key = cv::waitKey(10);
		if (key == 'q') break;
		else if (key == 32) stop = !stop;

		if (stop)i -= 1;

		sensor.frameRelease();
	}
#ifdef LOG
	myfile.close()
#endif
	return 0;
}

