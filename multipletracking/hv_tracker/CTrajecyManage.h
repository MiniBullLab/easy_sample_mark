//角点跟踪
#ifndef CTRAJECT_H
#define CTRAJECT_H

#include "utility.h"
#include "CTrajecy.h"

typedef struct trajecyMangeRuntimeContext
{
	const cv::Mat* pSrcColImg;
	const TrackPair* pTrackpairs; //当前灰度图
	const vector<cv::Rect2f>* pLastRioRecs;// 上一帧的角点感兴趣区域。
	const cv::Mat* pSrcGrayImg;
	trajecyMangeRuntimeContext()
	{
		pSrcColImg = NULL;
		pTrackpairs = NULL;
		pLastRioRecs = NULL;
		pSrcGrayImg = NULL;
	}
}trajecyMangeRuntimeContext;

class  CTrajecyManage
{
public:
	CTrajecyManage(int nMaxTrajecNum);
	~CTrajecyManage();

public:

	// 轨迹处理入口函数
	void process(const trajecyMangeRuntimeContext* pTrackMagContext);

	//获得当前有效轨迹
	list<Ctracjecy*>* GetUsedTrajecy() { return &m_Usedtracjecy; }

	//当轨迹要被删除时，会将其“释放”，设置为闲置状态
	void Freetracjecy(Ctracjecy* ptracjecy);

	//删除轨迹
	void DelTrajectoys(bool bDelAll = false);
private:
	
	//轨迹更新
	void updataTrajectoy(const TrackPair* pTrackpairs);

	//新增轨迹
	void newTrajectoy(const Pointf &inputPoints, const Pointf &MatchPoints);

	
	//获得当前可用的轨迹
	Ctracjecy* applyFreetracjecy();


	//核对上一帧的轨迹都在当前帧被更新到（匹配或者匹配不上）
	bool CheckTrajecyMatchPair(const TrackPair* pTrackpairs);

#ifdef SHOW_TRAJECY_IMG
	void DrawTrajecys(const cv::Mat *pImg, float fscale)const;
#endif

private:
	Ctracjecy *m_pTrackjecyPool;//轨迹池子
	const cv::Mat *m_pImg;
	list<Ctracjecy *> m_Freetracjecy; //空闲的轨迹空间集合
	list<Ctracjecy *> m_Usedtracjecy;
	unsigned int  m_uCurId;//轨迹ID号
	int  m_nMaxTrajNum;
	
};

#endif