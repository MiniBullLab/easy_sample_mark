
//角点跟踪
#ifndef CORNERTRACK_H
#define CORNERTRACK_H

#include "utility.h"


typedef Pointf flowVec; //运动矢量
#define MAX_DENSE_RIO_NUM 8 //运行最多的稠密跟踪的RIO

//基于点跟踪的运行上下文
typedef struct CornertrackRuntimeContext
{
	const cv::Mat* pSrcColImg;
	const cv::Mat* pCurGrayImg; //当前灰度图
	const cv::Mat* preGrayImg; //上一帧灰度图
	const vector<Pointf> *pcorners; //上一帧提取到的角点数
	const vector<cv::Rect2f>* pTrackRios; //上一帧跟踪的感兴趣区域
	const cv::Rect* pCurTrackRio; //当前帧跟踪的感兴趣区域
	CornerTrackType tracktype; //跟踪类型
	vector<CornerTrackFilterType> FilterType; //跟踪点过滤类型
	

	CornertrackRuntimeContext()
	{
		pCurGrayImg = NULL;
		preGrayImg = NULL;
		pTrackRios = NULL;
		pcorners = NULL;
		tracktype = DENSE_LK;
		pCurTrackRio = NULL;
	}
	
}CornertrackRuntimeContext;



class  CornerTrack
{
public:
	CornerTrack();
	~CornerTrack();

public:
	bool CornerTrackProcess(const CornertrackRuntimeContext* pCornerTracRuntime);//角点跟踪
	const TrackPair* GetCornerTrackResult() { return &m_TrackMatchPoints; }
	const TrackPair* GetLanMarkTrackResult() { return &m_LandkMatchPoints; }
private:
	//稠密光流
	bool DenseOpticaltrack(const CornertrackRuntimeContext* pCornerTracRuntime,\
		TrackPair *pTrackMatchPoints);

	//稀疏光流
	bool SparseOpticaltrack(const CornertrackRuntimeContext* pCornerTracRuntime,\
		TrackPair *pTrackMatchPoints);

	//匹配角点Filter
	void FilterTrack(const CornertrackRuntimeContext* pCornerTracRuntime, TrackPair* pMatchPoints);

	//计算全局的主运动矢量
	flowVec CalMajorFlowvector(cv::Mat *pFlowMat);

	//fliter1_方向：
	void FilterDir(const CornertrackRuntimeContext* pCornerTracRuntime, \
		TrackPair* pMatchPoints, cv::Mat *pFlowMat,int dirFilter = 1);

	//fliter2_Ransac：
	void FilterRanSac(TrackPair* pMatchPoints);

	void SplitLpsFromMatcs(TrackPair* pMatchPoints, TrackPair* pLandMarks);

#ifdef SHOW_IMG
	//显示匹配结果
	void ShowMatchResult(const CornertrackRuntimeContext* pCornerTracRuntime, \
		TrackPair* pMatchPoints);
#endif


private:
	const CornertrackRuntimeContext* m_pCornerTracContext; //2帧直接的角点跟踪的上下文
	TrackPair m_TrackMatchPoints;
	TrackPair m_LandkMatchPoints;
	cv::Mat *m_pFlowMat;
	
};


#endif