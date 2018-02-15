//#pragma once
//
//class Labeling {
//public:
//	typedef struct {
//		double x;	//x average
//		double y;	//y average
//		double d;	//depth
//		double width; //width
//		double height; //height
//		double size;   //size
//		int id;		   //id
//	}person;
//
//	std::vector<person> results;//labeling
//								  //std::set<int>isId;
//	void labeling(cv::Mat depth);//
//								 //void drawResult(const std::string&winname);
//private:
//	int id;
//	cv::Mat table;	//
//
//	typedef struct {
//		short x;
//		short y;
//	}Buf;//pointバッファー
//
//		 //取得メソッド
//	int getNotLabelDepth(cv::Mat depth, cv::Mat table, int x, int y) {
//		if (table.at<short>(y, x) == INIT)return depth.at<short>(y, x);
//		return 0;
//	}
//	//代入メソッド
//	labelInf getlabelInf(double x, double y, double z, double width, double height, double size, int id) {
//		labelInf tmp = { x, y, z, width, height, size, id };
//		return tmp;
//	}
//	Buf getBufInf(short x, short y) {
//		Buf tmp = { x,y };
//		return tmp;
//	}
//};
