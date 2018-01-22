#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <set>
#include "Labeling.hpp"
#include "KinectBase.hpp"
#include "Global.hpp"
#include "KinectParam.hpp"

using namespace lab;

Parameter param;

std::set<int>isId;
std::vector<cv::Vec3d> bestInliers;
void Labeling::labeling(cv::Mat depth,int step)
{
	KinectSensor kinect;

	id = 0;
	results.clear();
	isId.clear();

	table = cv::Mat::zeros(KINECT_HEIGHT,KINECT_WIDTH,CV_16UC1);//初期化
	//ラベリングの捜査が終了していない箇所はINITの値が入るようにする
	for (int y = step; y < KINECT_HEIGHT - step; y++) {
		for (int x = step; x < KINECT_WIDTH - step; x++) {
			table.at<short>(y, x) = INIT;
		}
	}

	//平面除去
	Plane3D plane;
	param.get_ransac(0, depth, plane, bestInliers);
	for (int i = 0; i<bestInliers.size(); i++) {
		cv::Point3d p(bestInliers.at(i));
		cv::Point3d imgP;
		if (kinect.worldToDepth(p, imgP)<0) continue;
		for (int j = -5; j <= 5; j++) {
			for (int k = -5; k <= 5; k++) {
				if ((imgP.x + k) < KINECT_WIDTH && 0 < (imgP.x + k) && 0<(imgP.y + j) && (imgP.y + j)<KINECT_HEIGHT) {
					table.at<short>(imgP.y + j, imgP.x + k) = INIT + 1;
				}
			}
		}
	}
	//ラベリング
	for (int y = step+10; y < KINECT_HEIGHT-step-10; y+=step){
		for (int x = step+10; x < KINECT_WIDTH-step-10; x+=step){
			if (depth.at<short>(y, x) != 0 && table.at<short>(y, x) == INIT){
				////床の近くに機体があるとする
				//bool out = true;
				//for (int i = -10; i < 10; i++) {
				//	for (int j = -10; j < 10; j++) {
				//		if (table.at<short>(y + i, x + j) == INIT + 1)out=false;
				//	}
				//}
				//if (out)continue;

				//
				std::vector<Buf> pointBuf;//
				Labeling::labelInf labelBuf = { 0 };//ラベリング結果中間結果
				pointBuf.push_back(getBufInf(x,y));	//点の追加
				while (!pointBuf.empty()){//
					double ty = (pointBuf.back()).y, tx = (pointBuf.back()).x;
					pointBuf.pop_back();
					table.at<short>(ty,tx) = id;
					int d = depth.at<short>(ty, tx);
					labelBuf = getlabelInf(labelBuf.x + x, labelBuf.y + ty, labelBuf.d + d, labelBuf.size+1,id);//

					for (int i = 0; i < 4; i++) {
						double ret = getNotLabelDepth(depth, table, tx + X[i], ty + Y[i]);//4傍点
						if (abs(ret - d) < RELATION_DEPTH && table.at<short>(ty + Y[i], tx + X[i])==INIT) {//差分が近ければ一つのものとみなす
							pointBuf.push_back(getBufInf(tx + X[i], ty + Y[i]));
						}
					}
				}
				if (labelBuf.size > MIN_SIZE && labelBuf.size < MAX_SIZE) {
					//ラベリング成功の場合
					labelBuf = getlabelInf(labelBuf.x / labelBuf.size, labelBuf.y / labelBuf.size, labelBuf.d / labelBuf.size, labelBuf.size,id);
					results.push_back(labelBuf);//最終結果をresultに入れる
					isId.insert(id);
					id++;//idを更新
				}
			}
		}
	}
}

void Labeling::drawResult(const std::string&winname)
{
	KinectSensor kinect;
	cv::Mat paintImg = cv::Mat::zeros(KINECT_HEIGHT, KINECT_WIDTH, CV_8UC3);
	//	cv::cvtColor(table, paintImg, CV_GRAY2BGR);

	if (table.empty())return;//エラー処理

	//描画(目がちかちかするからほかの方法も考える!
	for (int y = 0; y < KINECT_HEIGHT; y++) {
		for (int x = 0; x < KINECT_WIDTH; x++) {
			int num = table.at<short>(y, x);
			if (num != INIT && isId.find(num)!=isId.end() && num !=INIT+1) {
				paintImg.at<cv::Vec3b>(y, x)[0] = 255 * (num % 2);
				paintImg.at<cv::Vec3b>(y, x)[1] = (255 / 4)*(num % 4);
				paintImg.at<cv::Vec3b>(y, x)[2] = (255 / 2)*(num % 3);
			}
		}
	}
	for (int i = 0; i<bestInliers.size(); i++) {
		cv::Point3d p(bestInliers.at(i));
		cv::Point3d imgP;
		if (kinect.worldToDepth(p, imgP)<0) continue;
		cv::circle(paintImg, cv::Point(imgP.x, imgP.y), 1, cv::Scalar(255, 0, 0), -1);
	}

	cv::imshow(winname, paintImg);
}