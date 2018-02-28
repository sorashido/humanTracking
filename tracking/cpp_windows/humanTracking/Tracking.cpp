#include "Tracking.hpp"

void Track::trackPeople(std::vector<personInf>* people, std::vector<std::vector<personInf>>* track_data)
{
	bool isadd = false;
	for (auto p : *people) {
		for (auto t = track_data->begin(); t != track_data->end(); ++t) {
			personInf tmp = t->back();
			double rate = sqrt(p.wx*p.wx + p.wz*p.wz) / 5;
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
			track_data->push_back(per);
		}
	}
}
