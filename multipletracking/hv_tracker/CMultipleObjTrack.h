#ifndef CMULTIPLEOBJTRACK_H
#define CMULTIPLEOBJTRACK_H

#include "utility.h"
#include "opencv2/opencv.hpp"

class CTrack4Corner;
class CObjgroup;
class Cobject;
struct CobjectRuntimeContext;
struct TrackRuntimeContext;

typedef struct trackPro
{
	CornerType cornertype; //角点类型
	CornerTrackType TrackType; //跟踪类型
	const cv::Mat* pMask; //mask图
	vector<Pointf> AddPoints;
	vector<CornerTrackFilterType> FilterType; //跟踪点过滤类型
	int nMaxCornerNum; //每帧最多获得的角点个数
	int nMaxTrajecyNum;//最多跟踪的轨迹数目
	trackPro()
	{
		cornertype = Fast;
		TrackType = SPARSE_LK;
		nMaxCornerNum = 64;
		nMaxTrajecyNum = 64;
		pMask = NULL;
	}
}trackPro;

typedef struct MutilTrackContext
{
	const cv::Mat* pSrcColImg;//彩色图像
	const cv::Mat* pGrayImg; //灰度图
	const cv::Mat* preGrayImg; //上一帧目标图像
	double dTimestamp; //当前时间
	vector<Blob> Blobs;//当前帧检测到的感兴趣目标区域
	bool bUniqueObj; //目标唯一性:仅仅跟踪一个目标

	trackPro TrackPro; //跟踪配置属性
	MutilTrackContext()
	{
		pSrcColImg = NULL;
		pGrayImg = NULL;
		preGrayImg = NULL;
		bUniqueObj = false;
	}
}MutilTrackContext;

class  CMultipleObjTrack
{
public:
	CMultipleObjTrack();
	~CMultipleObjTrack();

public:

	//单/多目标跟踪接口，预测跟踪结果
	void MuptileTrackProcess(const cv::Mat *pImg,
		double dTsCur, bool bBigFaceScene = true, \
		bool bEvlTrack = false,
		CornerType cornertype = Fast, \
		CornerTrackType TrackType = SPARSE_LK,
		int nMaxCornerNum = MAX_CORNER_NUM,
		int nMaxTrajecyNum = MAX_TRAJECY_NUM);

	//多个检测目标的创建
	void MuptileTrackobjCreter(const vector<cv::Rect> &rectVector, const vector<float> &fconf);

	//单个检测目标的创建
	void SingleTrackobjCreter(cv::Rect Detrect, float fConfidence, cv::Rect &fuseRec,
		vector<cv::Point>* pvLandMark = NULL);

	const list<Cobject*> GetMatureTrackObjs();

	vector<Pointf> GetLandMarkTrack();

private:

	const list<Cobject*>* GetTrackObjs();

	void SetTrackRuntimeContext(TrackRuntimeContext* pTrackcontext, double dTsCur);

	void SaveTrackResult();

	//配置上下文
	void SetMulObjTrackcontext(const cv::Mat*pImg,
		double dTsCur, bool bBigFaceScene = true, \
		CornerType cornertype = Fast, \
		CornerTrackType TrackType = SPARSE_LK,
		int nMaxCornerNum = MAX_CORNER_NUM, //有一部分来自landmark（14)
		int nMaxTrajecyNum = MAX_TRAJECY_NUM);

	//设置目标跟踪上下文
	void SetobjectRuntimeContext( \
		const MutilTrackContext *pMulTrackContext, \
		CobjectRuntimeContext* pTrackObjContext);

	void MuptileTrackInit(int nMaxTrajecNum = 64);

	cv::Rect2f RestirctRect(cv::Rect2f Rect, cv::Size size);

	void GetfuseRec(cv::Rect Detrect, float fConfidence,cv::Rect &fuseRec);

	void SetLandMark(vector<cv::Point>* pvLandMark);

#ifdef SHOW_IMG
	void ShowDetAndtrackLandMark(vector<cv::Point>* pvLandMark);

	void TrackAndDetFuseRes();

	void CloseAllShowWindow();
#endif

private:
	CTrack4Corner* m_pCornerTrack;//主要通过2帧的匹配，获得轨迹信息
	CObjgroup* m_ObjGroup; //通过轨迹进行目标的预测
	MutilTrackContext *m_pMulTrackContext;
	vector<cv::Point> m_vLandMark;
	cv::Mat m_CurGray; //当前帧图像数据
	cv::Mat  m_preGray;  //上一帧图像数据
	unsigned int m_uFramSeq;
	uchar* m_pCurdata;
	uchar* m_predata;
	uchar* m_pTempdata;
public:
	float m_fscale;
};

#endif
