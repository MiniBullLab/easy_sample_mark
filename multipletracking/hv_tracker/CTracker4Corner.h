#ifndef  CTRACKERRFORCORNER_H 
#define CTRACKERRFORCORNER_H

#include "utility.h"


struct CornerFindRuntimeContext;
struct CornertrackRuntimeContext;
struct trajecyMangeRuntimeContext;
class CornerFind;
class CornerTrack;
class CTrajecyManage;
class Ctracjecy;


//点提取的运行上下文
typedef struct TrackRuntimeContext
{
	const cv::Mat* pSrcColImg;//彩色图像
	const cv::Mat* pGrayImg; //灰度图
	const cv::Mat* preGrayImg; //上一帧目标图像
	const cv::Mat* pMask; //mask图
	vector<cv::Rect2f> LastRioRecvec;//感兴趣区域
	CornerType cornertype; //提取角点类型
	CornerTrackType TrackType; //跟踪类型
	vector<Pointf> AddPoints;//补充的角点(如人脸上的mask点)
	vector<CornerTrackFilterType> FilterType; //跟踪点过滤类型
	int nMaxCornerNum;
	int nMaxTrajecyNum;
	TrackRuntimeContext()
	{
		pGrayImg = NULL;
		preGrayImg = NULL;
		pMask = NULL;
		cornertype = Goodfeature;
		TrackType = DENSE_LK;  
		nMaxCornerNum = 64;
		nMaxTrajecyNum = 64;
	}

}TrackRuntimeContext;



class  CTrack4Corner
{
public:
	CTrack4Corner(int nMaxTrajecNum);
	~CTrack4Corner();

public:

	void init(const  TrackRuntimeContext* pTrackContext); //初始化函数

	void uninit(); //释放函数

	void Reset(); //重置函数

    void Process(const TrackRuntimeContext* pTrackContext); //基于角点的跟踪主入口

	list<Ctracjecy*>* GetUsedTrajecy();
	CTrajecyManage *GetTraTrajecyManage() { return m_pTrajecyManage;}

	const TrackPair* GetLandMarkTrackPair();

private:

	//设置角点提取和角点跟踪的上下文
	void SetContext(const  TrackRuntimeContext* pTrackContext,CornerFindRuntimeContext* pCornerFidContex, \
		CornertrackRuntimeContext* pCornerTrackContex,\
		trajecyMangeRuntimeContext* ptrajecyMangContex);
	void AddTrajecyPoint(vector<Pointf> *pAddPoints);

	
private:

	CornerFind* m_pCornerFind; //角点提取模块

	CornerTrack* m_pCornerTrack; //角点跟踪模块

	CTrajecyManage *m_pTrajecyManage; //轨迹形成管理模块


};


#endif
