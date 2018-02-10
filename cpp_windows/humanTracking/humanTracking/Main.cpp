#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <random>
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
	double z;
	double width;
	double height;
	int num;
	int frame;
	int id;
}personInf;

int main(){
	DepthSensor sensor(L"D:\\track_data\\No6_out2017-11-03 6-08-56.rssdk");

	Labeling label;
	cv::Mat depthMat(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);

	std::vector<personInf> people;//
	std::vector<std::vector<personInf>> track_data;

	static Point3D cameraPoint, worldPoint;

	// window setting
	const static std::string WINDOWNAME = "Depth";
	cv::namedWindow(WINDOWNAME);
	cv::setMouseCallback(WINDOWNAME, onMouse);

	int view_mode = 0;

	std::random_device rnd;

	ofstream myfile;
	myfile.open("example.csv");
	
	// Streaming loop
	for (int i = 6500; i < 200000; i += 1) {
		sensor.getFrame(i, &depthMat);

		label.labeling(depthMat);

		Mat trackMat = Mat::zeros(cv::Size(640, 480), CV_8UC3);

		//
		Point3D c1, c2, w1, w2;
		int id = 0;
		people.clear();
		std::set<int>isId;

		for (auto r1 : label.results) {
			c1.x = r1.x;
			c1.y = r1.y;
			c1.z = r1.d;
			sensor.cameraToWorldPoint(&c1, &w1);

			personInf personBuf;
			personBuf.x = w1.x;
			personBuf.y = w1.y;
			personBuf.z = w1.z;
			personBuf.height = r1.height;
			personBuf.width = r1.width;
			personBuf.frame = i;
			personBuf.id = id;
			personBuf.num = 1;
			for (auto r2 : label.results) {
				if (r1.id == r2.id)continue;
				c2.x = r2.x;
				c2.y = r2.y;
				c2.z = r2.d;
				sensor.cameraToWorldPoint(&c2, &w2);
				if (sqrt(abs(w1.x - w2.x)*abs(w1.x - w2.x)) < 100 && sqrt(abs(w1.z - w2.z)*abs(w1.z - w2.z)) <  400) {
					personBuf.x += w2.x;
					personBuf.y += w2.y;
					personBuf.z += w2.z;
					personBuf.num += 1;
					personBuf.height += r2.height;
					personBuf.width += r2.width;
					isId.insert(r2.id);
				}
			}

			if (isId.find(r1.id) == isId.end() && personBuf.z / 4000 * personBuf.height > 30) {
				isId.insert(r1.id);
				personBuf.x /= personBuf.num;
				personBuf.y /= personBuf.num;
				personBuf.z /= personBuf.num;
				people.push_back(personBuf);
				id++;
			}
		}

		bool isadd = false;
		std::vector<personInf> tt;
		for (auto p : people) {
			for (auto t = track_data.begin(); t != track_data.end(); ++t){
				personInf tmp = t->back();
				if (sqrt(abs(tmp.x - p.x)*abs(tmp.x - p.x)) < 30 && sqrt(abs(tmp.z - p.z)*abs(tmp.z - p.z)) < 100) {
					t->push_back(p);
					isadd = true;
				}
			}
			for (auto t : track_data) {
				if (i - t.back().frame > 100) {
					t.clear();
				}
			}

			if (!isadd) {
				std::vector<personInf> per;
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

		//sprintf_s(str, "%4d, %4d, %4d", (int)world[m_y/rate * 320 + m_x/rate].x, (int)world[m_y/rate * 320 + m_x/rate].y, (int)world[m_y/rate * 320 + m_x/rate].z);
		sprintf_s(str, "%4d", (int)depthMat.at<short>(m_y/rate, m_x/rate));
		cv::putText(paintMat, str, cv::Point(m_x, m_y), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(7, 193, 255), 2, CV_AA);

		switch (view_mode) {
		case 9:
			// stop
			i -= 1;
			break;
		default:
			break;
		}

		for (auto r : people) {
			w1.x = r.x;
			w1.y = r.y;
			w1.z = r.z;
			sensor.worldToCameraPoint(&w1, &c1);
			cv::rectangle(paintMat, Point(c1.x*rate - r.width*rate / 2, c1.y*rate - r.height*rate / 2), Point(c1.x*rate + r.width*rate / 2, c1.y*rate + r.height*rate / 2), Scalar(136, 150, 0), 2);
			sprintf_s(str, "%4d", (int)r.id);
			cv::putText(paintMat, str, cv::Point(c1.x*rate, c1.y*rate), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(54, 67, 244), 2, CV_AA);

			myfile << r.x << "," << r.y << "," << r.z << "," << r.frame << "\n";
		}
		//for (auto r : people) {
		//	cameraPoint.x = r.x;
		//	cameraPoint.y = r.y;
		//	cameraPoint.z = r.z;
		//	sensor.cameraToWorldPoint(&cameraPoint, &worldPoint);
		//	cv::circle(trackMat, cv::Point(worldPoint.x * rate, -worldPoint.z * 480 / 4000 + 650), 5, cv::Scalar(54, 67, 244));
		//}
		int color = 0;
		for (auto t : track_data) {
			for (auto r : t) {
				if (i - r.frame < 40) {
					cv::circle(trackMat, cv::Point(r.x * rate, -r.z * 480 / 4000 + 650), 5, cv::Scalar(color%255, 67, 244));
					//sprintf_s(str, "%4d", (int)color/30);
					//cv::putText(trackMat, str, cv::Point(r.x * rate, -r.z * 480 / 4000 + 650), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(54, 67, 244), 2, CV_AA);
				}
				//sprintf_s(str, "%4d", (int)r.id);
				//cv::putText(trackMat, str, cv::Point(r.x * rate, -r.z * 480 / 4000 + 650), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(54, 67, 244), 2, CV_AA);
			}
			color += 30;
		}
		cv::imshow("track", trackMat);
		cv::imshow(WINDOWNAME, paintMat);
		int key = cv::waitKey(10);
		if (key >= '0' && key <= '9') view_mode = key - '0';
		else if (key == 'q') break;

		sensor.frameRelease();
	}
	myfile.close();

	return 0;
}

