#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <set>
#include "Detecting.hpp"
#include "DepthSensor.hpp"

using namespace labelParam;
std::vector<cv::Vec3d> bestInliers;
void Detect::detectPeople(DepthSensor* sensor, int frame, cv::Mat depth, PXCPoint3DF32* vertices, std::vector<personInf>* people)
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

	// Detect
	for (int y = step; y < DEPTH_HEIGHT - step; y += step){
		for (int x = step; x < DEPTH_WIDTH - step; x += step){
			if (depth.at<short>(y, x) != 0 && table.at<short>(y, x) == INIT){
				std::vector<Buf> pointBuf;
				Detect::labelInf labelBuf = { 0 };//point buffer

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
					labelBuf = getlabelInf(labelBuf.sx + tx, labelBuf.sy + ty, labelBuf.sd + d, labelBuf.cx + cx, labelBuf.cy + cy, labelBuf.cz + cz, maxw - minw, maxh - minh , labelBuf.size+1, id);
					//labelBuf = getlabelInf(labelBuf.x + cx, labelBuf.y + cy, labelBuf.d + cz, maxw - minw, maxh - minh , labelBuf.size+1, id);

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
						labelBuf = getlabelInf(labelBuf.sx / labelBuf.size, labelBuf.sy / labelBuf.size, labelBuf.sd / labelBuf.size, 
												labelBuf.cx / labelBuf.size, labelBuf.cy / labelBuf.size, labelBuf.cz / labelBuf.size, 
												labelBuf.width, labelBuf.height, labelBuf.size, id);
						results.push_back(labelBuf);
				}
				id += 1;
			}
		}
	}

	//
	Point3D c1, c2, w1, w2;
	std::set<int>isId;

	for (auto r1 : results) {
		c1.x = r1.cx;// vertices[r1.y * 320 + r1.x].x;
		c1.y = r1.cy;// vertices[r1.y * 320 + r1.x].y;
		c1.z = r1.cz;
		sensor->cameraToWorldPoint(&c1, &w1);

		personInf personBuf;
		personBuf.wx = w1.x;
		personBuf.wy = w1.y;
		personBuf.wz = w1.z;
		personBuf.sx = r1.sx;
		personBuf.sy = r1.sy;
		personBuf.sz = r1.sd;
		personBuf.height = r1.height;
		personBuf.width = r1.width;
		personBuf.frame = frame;
		personBuf.id = 0;
		personBuf.num = 1;
		for (auto r2 : results) {
			if (r1.id == r2.id)continue;
			c2.x = r2.cx;
			c2.y = r2.cy;
			c2.z = r2.cz;
			sensor->cameraToWorldPoint(&c2, &w2);
			if (sqrt(abs(w1.x - w2.x)*abs(w1.x - w2.x)) < 300 && sqrt(abs(w1.z - w2.z)*abs(w1.z - w2.z)) < 300) {
				personBuf.wx += w2.x;
				personBuf.wy += w2.y;
				personBuf.wz += w2.z;
				personBuf.sx += r1.sx;
				personBuf.sy += r1.sy;
				personBuf.sz += r1.sd;
				personBuf.num += 1;
				personBuf.height += r2.height;
				personBuf.width += r2.width;
				isId.insert(r2.id);
			}
		}

		if (isId.find(r1.id) == isId.end() && personBuf.wz / 4000 * personBuf.height > 30) {
			isId.insert(r1.id);
			personBuf.wx /= personBuf.num;
			personBuf.wy /= personBuf.num;
			personBuf.wz /= personBuf.num;
			personBuf.sx /= personBuf.num;
			personBuf.sy /= personBuf.num;
			personBuf.sz /= personBuf.num;
			people->push_back(personBuf);
		}
	}
}