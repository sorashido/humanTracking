#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <set>
#include "Labeling.hpp"
#include "DepthSensor.hpp"

using namespace labelParam;
std::vector<cv::Vec3d> bestInliers;
void Labeling::labeling(cv::Mat depth, PXCPoint3DF32* vertices)
{
	int step = 1;

	id = 1;
	results.clear();

	table = cv::Mat::zeros(DEPTH_HEIGHT, DEPTH_WIDTH, CV_16UC1);//init

	//set init value
	for (int y = step; y < DEPTH_HEIGHT - step; y++) {
		for (int x = step; x < DEPTH_WIDTH - step; x++) {
			table.at<short>(y, x) = INIT;
		}
	}

	// labeling
	for (int y = step; y < DEPTH_HEIGHT - step; y += step){
		for (int x = step; x < DEPTH_WIDTH - step; x += step){
			if (depth.at<short>(y, x) != 0 && table.at<short>(y, x) == INIT){
				std::vector<Buf> pointBuf;
				Labeling::labelInf labelBuf = { 0 };//point buffer

				double maxw = x, minw = x, maxh = y, minh = y;
				pointBuf.push_back(getBufInf(x,y));	//add point

				while (!pointBuf.empty()){
					double ty = (pointBuf.back()).y, tx = (pointBuf.back()).x;
					pointBuf.pop_back();
					table.at<short>(ty, tx) = id;
					int d = depth.at<short>(ty, tx);

					double cx, cy, cz = 0;
					cx = vertices[(int)ty * 320 + (int)tx].x;
					cy = vertices[(int)ty * 320 + (int)tx].y;
					cz = vertices[(int)ty * 320 + (int)tx].z;

					if (tx > maxw)maxw = tx; if (tx < minw)minw = tx;
					if (ty > maxh)maxh = ty; if (ty < minh)maxh = ty;
					//labelBuf = getlabelInf(labelBuf.x + x, labelBuf.y + ty, labelBuf.d + d, maxw - minw, maxh - minh , labelBuf.size+1, id);
					labelBuf = getlabelInf(labelBuf.x + cx, labelBuf.y + cy, labelBuf.d + cz, maxw - minw, maxh - minh , labelBuf.size+1, id);

					for (int i = 0; i < SERCH_NUM; i++) {
						double ret = getNotLabelDepth(depth, table, tx + X[i], ty + Y[i]);
						if (abs(ret - d) < ((ret + d)/2*RELATION_RATE) && table.at<short>(ty + Y[i], tx + X[i])==INIT && depth.at<short>(ty + Y[i], tx + X[i]) != 0) {
							pointBuf.push_back(getBufInf(tx + X[i], ty + Y[i]));
						}
					}
				}

				// filter by size, width, height
				if (labelBuf.size > MIN_SIZE && labelBuf.size < MAX_SIZE
					&& labelBuf.width > MIN_WIDTH && labelBuf.width < MAX_WIDTH
					&& labelBuf.height > MIN_HEIGHT && labelBuf.height < MAX_HEIGHT) {
						labelBuf = getlabelInf(labelBuf.x / labelBuf.size, labelBuf.y / labelBuf.size, labelBuf.d / labelBuf.size, labelBuf.width, labelBuf.height, labelBuf.size, id);
						results.push_back(labelBuf);
				}
				id += 1;
			}
		}
	}
}