#pragma once

#include "DepthSensor.hpp"
#include "Detecting.hpp"

class Track {
public:
	Track() {
		people_num = 0;
	}
	int people_num;
	void trackPeople(std::vector<personInf>* people, std::vector<std::vector<personInf>>* track_data);
private:
};