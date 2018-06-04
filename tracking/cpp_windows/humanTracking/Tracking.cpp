#include <iostream>
#include "Tracking.hpp"

void Track::trackPeople(std::vector<detection>* people, std::vector<std::vector<detection>>* track_data)
{
	//update tracker
	bool isadd = false;
	for (auto p : *people) {
		for (auto t = track_data->begin(); t != track_data->end(); ++t) {
			detection tmp = t->back();
			double rate = sqrt(p.wx*p.wx + p.wz*p.wz) / 5;
			if (sqrt(abs(tmp.wx - p.wx)*abs(tmp.wx - p.wx) + abs(tmp.wz - p.wz)*abs(tmp.wz - p.wz)) < rate && (p.frame - tmp.frame) < 10) {
				p.id = t->back().id;
				t->push_back(p);
				isadd = true;
			}
		}

		if (!isadd) {
			std::vector<detection> per;
			p.id = people_num;
			people_num += 1;
			per.push_back(p);
			track_data->push_back(per);
		}

		if (track_data->size() > 100) {
			track_data->erase(track_data->begin());
		}
	}
	//add new tracker

}
