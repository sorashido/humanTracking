#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <set>
#include "Labeling.hpp"
#include "DepthSensor.hpp"

//#include "KinectBase.hpp"
//#include "Global.hpp"
//#include "KinectParam.hpp"

using namespace lab;

//Parameter param;

std::vector<cv::Vec3d> bestInliers;
void Labeling::labeling(cv::Mat depth, int step)
{
	//KinectSensor kinect;
	id = 1;
	results.clear();
	isId.clear();

	table = cv::Mat::zeros(DEPTH_HEIGHT,DEPTH_WIDTH,CV_16UC1);//初期化
	//ラベリングの捜査が終了していない箇所はINITの値が入るようにする
	for (int y = step; y < DEPTH_HEIGHT - step; y++) {
		for (int x = step; x < DEPTH_WIDTH - step; x++) {
			table.at<short>(y, x) = INIT;
		}
	}

	//ラベリング
	for (int y = step; y < DEPTH_HEIGHT - step; y += step){
		for (int x = step; x < DEPTH_WIDTH - step; x += step){
			if (depth.at<short>(y, x) != 0 && table.at<short>(y, x) == INIT){
				std::vector<Buf> pointBuf;//
				Labeling::labelInf labelBuf = { 0 };//ラベリング結果中間結果

				double maxw = x, minw = x, maxh = y, minh = y;
				pointBuf.push_back(getBufInf(x,y));	//点の追加

				while (!pointBuf.empty()){//
					double ty = (pointBuf.back()).y, tx = (pointBuf.back()).x;
					pointBuf.pop_back();
					table.at<short>(ty,tx) = id;
					int d = depth.at<short>(ty, tx);

					if (tx > maxw)maxw = tx; if (tx < minw)minw = tx;
					if (ty > maxh)maxh = ty; if (ty < minh)maxh = ty;
					labelBuf = getlabelInf(labelBuf.x + x, labelBuf.y + ty, labelBuf.d + d, maxw - minw, maxh - minh , labelBuf.size+1, id);//

					for (int i = 0; i < 4; i++) {
						double ret = getNotLabelDepth(depth, table, tx + X[i], ty + Y[i]);//4傍点
						if (abs(ret - d) < RELATION_DEPTH && table.at<short>(ty + Y[i], tx + X[i])==INIT && depth.at<short>(ty + Y[i], tx + X[i]) != 0) {//差分が近ければ一つのものとみなす
							pointBuf.push_back(getBufInf(tx + X[i], ty + Y[i]));
						}
					}
				}
				//
				if (labelBuf.size > MIN_SIZE && labelBuf.size < MAX_SIZE //){
					&& labelBuf.width > MIN_WIDTH && labelBuf.width < MAX_WIDTH
					&& labelBuf.height > MIN_HEIGHT && labelBuf.height < MAX_HEIGHT) {
						labelBuf = getlabelInf(labelBuf.x / labelBuf.size, labelBuf.y / labelBuf.size, labelBuf.d / labelBuf.size, labelBuf.width, labelBuf.height, labelBuf.size, id);
						results.push_back(labelBuf);//最終結果をresultに入れる
						isId.insert(id);
						id += 1;//idを更新
				}
			}
		}
	}
}

//void Labeling::drawResult(const std::string&winname)
//{
//	cv::Mat paintImg = cv::Mat::zeros(DEPTH_HEIGHT, DEPTH_WIDTH, CV_8UC3);
//	//	cv::cvtColor(table, paintImg, CV_GRAY2BGR);
//
//	if (table.empty())return;//エラー処理
//
//	//for (auto r : results) {
//	//	std::cout << r.x << "," << r.id <<"\n";
//	//}
//	//描画(目がちかちかするからほかの方法も考える!
//	for (int y = 0; y < DEPTH_HEIGHT; y++) {
//		for (int x = 0; x < DEPTH_WIDTH; x++) {
//			int num = table.at<short>(y, x);
//			if (num != INIT && isId.find(num) != isId.end() && num != INIT+1) {
//				paintImg.at<cv::Vec3b>(y, x)[0] = 255; //255 * (num % 2);
//					paintImg.at<cv::Vec3b>(y, x)[1] = 0; //(255 / 4)*(num % 4);
//					paintImg.at<cv::Vec3b>(y, x)[2] = 0; //(255 / 2)*(num % 3);
//			}
//		}
//	}
//	
//	//for (int i = 0; i<bestInliers.size(); i++) {
//	//	cv::Point3d p(bestInliers.at(i));
//	//	cv::Point3d imgP;
//	//	if (kinect.worldToDepth(p, imgP)<0) continue;
//	//	cv::circle(paintImg, cv::Point(imgP.x, imgP.y), 1, cv::Scalar(255, 0, 0), -1);
//	//}
//
//	cv::imshow(winname, paintImg);
//}