#include "CornerFind.h"
#include "opencv2/opencv.hpp"


bool KeypointDescend(cv::KeyPoint A, cv::KeyPoint B)
{
	return A.response > B.response;
}
CornerFind::CornerFind()
:m_CornerThresh(20)
{

}

CornerFind::~CornerFind()
{
	m_Corners.clear();
}
/*********************
函数名：FastCorner
输入参数：pCornerContext运行上下文
输出：无
功能：根据提取角点方式，寻址对应的角点提取策略

***********************/
void CornerFind::FindCorner(const CornerFindRuntimeContext* pCornerContext)
{
	//只有当有感兴趣时，才进行角点检测
	m_Corners.clear();
	
	//在提取角点之前CTrack4Corner::SetContext 已经将上一帧的轨迹点，和新增的补充点（如人脸点）已经添加！
	if (!pCornerContext->pRioRecs->empty() && 
		(int)pCornerContext->AddPoints.size() < pCornerContext->nMaxCornerNum)
	{
		//更加提取角点类型，进行对应的角点提取
		switch (pCornerContext->cornertype)
		{
			case Fast:
			{
				FastCorner(pCornerContext, m_Corners);
				break;
			}

			case Goodfeature:
			{		
				GoodFeaturCorner(pCornerContext, m_Corners);
				break;
			}
			case GRID:
			{				
				GridCorner(pCornerContext, m_Corners);
				break;
			}

			default:
				break;
			}

	}

#ifdef PRINTF_INF
	cout << "AddPoints:" << pCornerContext->AddPoints.size() << endl;
	cout << "m_Corners:" << m_Corners.size() << endl;
#endif
	
	//角点融合（所有的角点都是在上一帧图像得到的）m_Corners：是当前检测到的点
	//m_fuseCorners融合点
	Cornerfuse(pCornerContext->AddPoints, &m_Corners,&m_fuseCorners, pCornerContext->nMaxCornerNum);

#ifdef PRINTF_INF
	cout << "m_fuseCorners:" << m_fuseCorners.size() << endl;
#endif
}
/*********************
函数名：Cornerfuse
输入参数：pCornerContext运行上下文，pCorners：当前提取到的角点
输出：无
功能：上下文输入点（上一帧的轨迹点等）与提取到的融合

***********************/
void CornerFind::Cornerfuse(const vector<Pointf> &AddPoints, \
	vector<Pointf>* pCorners, vector<Pointf>* pfuseCorners, int nMaxPoints)
{
	//在AddPoints基础上再有条件的添加目标区域提取到的角点！
	*pfuseCorners = AddPoints;

	if (pCorners->empty())
	{	
		return;
	}

	vector<int> validCornIndex;
	const  int nMinDis = 3;

	vector<Pointf>::iterator  iter = pCorners->begin();

	//过滤到与输入点距离小于nMinDis的点
	for (; iter != pCorners->end();)
	{
		bool bfaraway = true;
		
		//bLandMark landMark点不参与轨迹，只做前后2帧的匹配
		if (iter->bLandMark)
		{
			iter++;
			continue;
		}
		for (int i = 0; i < (int)AddPoints.size(); i++)
		{
			int nDis = int(abs(iter->x - AddPoints[i].x) + abs(iter->y - AddPoints[i].y));

			//说明提取到的角点与输入的角点靠得很近，则丢弃该点
			if (nDis < nMinDis)
			{
				bfaraway = false;
				break;
			}
		}

		if (bfaraway)
		{
			iter++;
		}
		else
		{
			iter = pCorners->erase(iter);
		}

	}

	//原则上，在上一帧的基础上再添加提取的角点
	for (int i = 0; i < (int)pCorners->size(); i++)
	{
		if ( (int)pfuseCorners->size() >= nMaxPoints )
		{
			break;
		}
		pfuseCorners->push_back(pCorners->at(i));
	}
}

/*********************
函数名：FastCorner
输入参数：pCornerContext运行上下文
输出：无
功能：实现Fast的角点提取

***********************/
void CornerFind::FastCorner(const CornerFindRuntimeContext* pCornerContext, vector<Pointf> &dstCorners)
{
	assert(Fast == pCornerContext->cornertype);
	dstCorners.clear();

 	if (!pCornerContext->pRioRecs || pCornerContext->pRioRecs->empty())
	{
		return;
	}

	//stpe1:按平均分配
	vector<float> RioCornerRatioweight;
	RioCornerRatioweight.resize(pCornerContext->pRioRecs->size());


	//根据面积，计算各自的提取角点个数比重
	for (int m = 0; m < (int)pCornerContext->pRioRecs->size(); m++)
	{
		RioCornerRatioweight[m] = (float)1.0 /pCornerContext->pRioRecs->size();
	}

	//仅对感兴趣提取！
	cv::Point OffPoint(0, 0);

	for (int m = 0; m < (int)pCornerContext->pRioRecs->size(); m++)
	{
		const cv::Rect2f* pRect = &pCornerContext->pRioRecs->at(m);
	
		vector<cv::KeyPoint> vptf;
		
		cv::Rect Rio = cv::Rect(int(pRect->x + SHRINKRATIO * pRect->width), \
			int(pRect->y + SHRINKRATIO * pRect->height), int(pRect->width *(1 - 2 * SHRINKRATIO)), int(pRect->height *(1 - 2 * SHRINKRATIO))) & \
			cv::Rect(0, 0, pCornerContext->pGrayImg->cols, pCornerContext->pGrayImg->rows);

		OffPoint = cv::Point(Rio.x, Rio.y);

		if (MIN(Rio.width, Rio.height) < 10)
		{
			continue;
		}

		//RIo 提取角点
		cv::FAST((*pCornerContext->pGrayImg)(Rio), vptf, m_CornerThresh);
		int nTmepTrehsh = m_CornerThresh;
		if (vptf.size() < MAX_CORNER_NUM * 0.6f)
		{

			m_CornerThresh -= 5;
			m_CornerThresh = MAX(10, m_CornerThresh);
			if (nTmepTrehsh != m_CornerThresh)
			{
				cv::FAST((*pCornerContext->pGrayImg)(Rio), vptf, m_CornerThresh);
			}		
		}
		else if (vptf.size() >  MAX_CORNER_NUM)
		{
			m_CornerThresh +=2;
			m_CornerThresh = MIN(26, m_CornerThresh);
		}

		if ( !vptf.empty() )
		{
			sort(vptf.begin(), vptf.end(), KeypointDescend);

			//获得刚兴趣角点
			for (int i = 0; i < MIN(pCornerContext->nMaxCornerNum, vptf.size()) * RioCornerRatioweight[m]; i++)
			{
				dstCorners.push_back(Pointf(vptf[i].pt.x + OffPoint.x, vptf[i].pt.y + OffPoint.y));
			}
		}
	}
}


/*********************
函数名：GoodFeaturCorner
输入参数：pCornerContext运行上下文，dstCorners：输出的角点
输出：无
功能：实现GoodFeatur的角点提取

***********************/
void CornerFind::GoodFeaturCorner(const CornerFindRuntimeContext* pCornerContext, vector<Pointf> &dstCorners)
{
	assert(Goodfeature == pCornerContext->cornertype);

	dstCorners.clear();

	if (!pCornerContext->pRioRecs || pCornerContext->pRioRecs->empty())
	{
		return;
	}

	
	

	//stpe1:按面积分配点的分布个数
	vector<float> RioCornerRatioweight;
	RioCornerRatioweight.resize(pCornerContext->pRioRecs->size());



	//根据面积，计算各自的提取角点个数比重
	for (int m = 0; m < (int)pCornerContext->pRioRecs->size(); m++)
	{
		RioCornerRatioweight[m] = (float)1.0 / pCornerContext->pRioRecs->size();
	}

	//仅对感兴趣提取！
	double dMinDist = 15;
	cv::Point OffPoint(0, 0);
	for (int m = 0; m < (int)pCornerContext->pRioRecs->size(); m++)
	{
		const cv::Rect2f* pRect = &pCornerContext->pRioRecs->at(m);
	
		vector<cv::Point2f> vptf;

		cv::Rect Rio = cv::Rect( int(pRect->x + SHRINKRATIO * pRect->width), \
								 int(pRect->y + SHRINKRATIO * pRect->height),
								 int(pRect->width *(1 - 2 * SHRINKRATIO)),\
								 int(pRect->height *(1 - 2 * SHRINKRATIO)) )
						& cv::Rect( 0, 0, pCornerContext->pGrayImg->cols, pCornerContext->pGrayImg->rows );
		

		OffPoint = cv::Point(Rio.x, Rio.y);
	

		cv::Mat RioMat = (*pCornerContext->pGrayImg)(Rio);
	
		int maxCorners = int(pCornerContext->nMaxCornerNum * RioCornerRatioweight[m]);
		
		if (MIN(Rio.width, Rio.height) < 15)
		{
			continue;
		}
		

		cv::goodFeaturesToTrack(RioMat, vptf, \
			maxCorners, 0.0001, dMinDist,
			cv::noArray(), 3, false, 0.004);

		
	
		//获得刚兴趣角点
		for (int i = 0; i < vptf.size() * RioCornerRatioweight[m]; i++)
		{
			dstCorners.push_back(Pointf(vptf[i].x + OffPoint.x, vptf[i].y + OffPoint.y));
		}

	}

}

/*********************
函数名：GridCorner
输入参数：pCornerContext运行上下文；dstCorners：输出的角点
输出：无
功能：按网格化进行角度划分，进行角点提取

***********************/
void CornerFind::GridCorner(const CornerFindRuntimeContext* pCornerContext, vector<Pointf> &dstCorners)
{
	assert(GRID == pCornerContext->cornertype);
	dstCorners.clear();

	
	//仅对感兴趣提取！
	if (pCornerContext->pRioRecs)
	{
		const int nConsGridStep = 5;

		for (int m = 0; m < (int)pCornerContext->pRioRecs->size(); m++)
		{
			const cv::Rect2f* pRect = &pCornerContext->pRioRecs->at(m);
			

			//按网格划分提取角点
			for (int j = int(pRect->y) + nConsGridStep; j < int(pRect->y + pRect->height); j += nConsGridStep)
			{
				for (int i = int(pRect->x) + nConsGridStep; i < int(pRect->x + pRect->width); i += nConsGridStep)
				{
					dstCorners.push_back(Pointf((float)i, (float)j));

				}
			}

		}

	}


}