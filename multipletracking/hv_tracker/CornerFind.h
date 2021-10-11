//角点跟踪
#ifndef CORNERFIND_H
#define CORNERFIND_H

#include "utility.h"



//点提取的运行上下文
typedef struct CornerFindRuntimeContext
{
	const cv::Mat* pSrcColImg;
	const cv::Mat* pGrayImg; //灰度图
	const vector<cv::Rect2f> *pRioRecs;//目标区域
	CornerType cornertype; //角点类型
	const cv::Mat* pMask; //mask图
	vector<Pointf> AddPoints;
	int nMaxCornerNum; //每帧做多检测的角点个数
	CornerFindRuntimeContext()
	{
		pGrayImg = NULL;
		pRioRecs = NULL;
		cornertype = Goodfeature;
		pMask = NULL;
		nMaxCornerNum = 64;
	}

}CornerFindRuntimeContext;

class  CornerFind
{
public:
	CornerFind();
	~CornerFind();

public:

	//角点提取
	void FindCorner(const CornerFindRuntimeContext* pCornerContext);
	const vector<Pointf>* GetCorner() { return &m_fuseCorners; }


private:
	//fast 角点提取
	void FastCorner(const CornerFindRuntimeContext* pCornerContext, vector<Pointf> &dstCorners);

	//GoodFeatur角点提取
	void GoodFeaturCorner(const CornerFindRuntimeContext* pCornerContext, vector<Pointf> &dstCorners);

	//按网格化进行角度划分
	void GridCorner(const CornerFindRuntimeContext* pCornerContext, vector<Pointf> &dstCorners);
	
	//融合角点
	void Cornerfuse(const vector<Pointf> &AddPoints, \
		vector<Pointf>* pCorners, vector<Pointf>* pfuseCorners, int nMaxPoints);

private:
	int m_CornerThresh;
	vector<Pointf> m_Corners; //提取到的角点
	vector<Pointf> m_fuseCorners; //融合的角点{存储顺序：轨迹点+额外补充点+目标区域提取点}
};

#endif