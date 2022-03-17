#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <list>
#include <algorithm>
#include <assert.h>
#include "opencv2/opencv.hpp"

using namespace std;

#define CHECK_THROWCRASH(CRASH_OK,EXP) {if(CRASH_OK) do{cout<< #EXP<<endl; throw  #EXP;}while(0);}

//#define SAFEDEL_ARRAY(p) { if(p) delete [] p; p = NULL;}

// #define SAFEDEL(p) { if(p) delete p; p = NULL;}

//#define SHOW_IMG // 调试窗口
#ifdef SHOW_IMG
//#define SHOW_TRAJECY_IMG // 调试窗口
//#define CORNER_SHOW
//#define PRINTF_INF
//#define SAVERES
#endif
//#define SHOW_TIME
//#define CHECK_TRACK_RUN //检查跟踪模块是否有异常情况

#define SHRINKRATIO  0.15
#define TRACK_INTERVAL_FRAME 1 //
#define TIME_OUT_THRESH  5 // 超时时间阈值
#define MAX_CORNER_NUM 16 //(14 from landMark +16)
#define MAX_TRAJECY_NUM 16 //NOTE: MAX_TRAJECY_NUM <= MAX_CORNER_NUM

enum CornerType
{
	NONE = 0,
	Fast, //FAST提取角点
    Goodfeature,
	GRID //网格化
};

enum CornerTrackType
{
	DENSE_LK = 0, //稠密光流
	SPARSE_LK  //稀疏光流
};

enum CornerTrackFilterType
{
	NCC_FILTER = 0,  //区域子模块
	Dir_FILTER, //方向过滤
	RANSAC_FILTER
};

typedef struct Pointf
{
	float x;
	float y;
	int naddressId; //在轨迹存储空间的ID索引
	bool bLandMark;
	Pointf(float fx = 0, float fy = 0, int uId = -1, bool bLM = false )
	{
		x = fx;
		y = fy;
		naddressId = uId;
		bLandMark = bLM;
	}
}Pointf;

typedef  struct CornerMatch
{
   Pointf inputPoints;
   Pointf MatchPoints;
   bool  status;
}CornerMatch;

typedef  struct TrackPair
{
	vector<CornerMatch> CornerMatchRes;
}TrackPair;

typedef struct Blob
{
	cv::Rect rec;
	float fConf;
	Blob(cv::Rect r, float fd)
	{
		rec = r;
		fConf = fd;
	}

}Blob;

typedef struct MatchFactor
{
	float fscale;//尺度变换
	float fx; //偏移量
	float fy;
	bool bSucss;
	MatchFactor()
	{
		fscale = 1;
		fx = 0;
		fy = 0;
		bSucss = false;
	}

}MatchFactor;

namespace cv
{
	class Mat;

	template<typename _Tp> class Point_;
	typedef Point_<float> Point2f;

	//template<typename _Tp> class Rect_;
	template<typename _Tp> class Rect_;
	typedef Rect_<int> Rect;
}



#endif
