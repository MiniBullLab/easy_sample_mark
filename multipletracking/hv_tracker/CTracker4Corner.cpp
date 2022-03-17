#include "CTracker4Corner.h"
#include "CornerFind.h"
#include "CornerTrack.h"
#include "CTrajecyManage.h"
#include "opencv2/opencv.hpp"

CTrack4Corner::CTrack4Corner(int nMaxTrajecNum) :
m_pCornerFind(NULL), \
m_pCornerTrack(NULL)
{
	m_pCornerFind = new CornerFind();

	m_pCornerTrack = new CornerTrack();

	m_pTrajecyManage = new CTrajecyManage(nMaxTrajecNum);
}

CTrack4Corner::~CTrack4Corner()
{
	uninit();
}

//初始化函数//初始化函数
void CTrack4Corner::init(const  TrackRuntimeContext* pTrackContext)
{
	assert(NULL == m_pCornerFind);
	(void)(pTrackContext);

}

void CTrack4Corner::uninit()
{
    if(m_pCornerFind)
    {
        delete m_pCornerFind;
        m_pCornerFind = NULL;
    }
    if(m_pCornerTrack)
    {
        delete m_pCornerTrack;
        m_pCornerTrack = NULL;
    }
    if(m_pTrajecyManage)
    {
        delete m_pTrajecyManage;
        m_pTrajecyManage = NULL;
    }
}

void CTrack4Corner::Reset()
{

}

/*********************
函数名：SetCornerContext
输入参数：pCornerContext角点运行上下文（输出）,TrackRuntimeContext pTrackContext:跟踪上下文
         ptrajecyMangContex 轨迹管理上下文
输出：无
功能：根据跟踪上下文,配置角点提取和点跟踪运行上下文

***********************/
void CTrack4Corner::SetContext(const  TrackRuntimeContext* pTrackContext, CornerFindRuntimeContext* pCornerFidContex, \
	CornertrackRuntimeContext* pCornerTrackContex, \
	trajecyMangeRuntimeContext*ptrajecyMangContex)
{
	//设置点提取的上下文
	pCornerFidContex->pSrcColImg = pTrackContext->pSrcColImg;
	pCornerFidContex->pGrayImg = pTrackContext->preGrayImg;
	pCornerFidContex->nMaxCornerNum = pTrackContext->nMaxCornerNum;


	//在对上一帧图像进行角点检测之前，还先补充上一帧轨迹匹配上的点（上一帧轨迹点优先级最高）
	pCornerFidContex->AddPoints.clear();
	AddTrajecyPoint(&pCornerFidContex->AddPoints);

	//其次增加额外补充点（如人脸点）
	for (int i = 0; i < (int)pTrackContext->AddPoints.size(); i++)
	{
		pCornerFidContex->AddPoints.push_back(pTrackContext->AddPoints.at(i));
	}

	pCornerFidContex->pMask = pTrackContext->pMask;
	pCornerFidContex->pRioRecs = &pTrackContext->LastRioRecvec;
	pCornerFidContex->cornertype = pTrackContext->cornertype;

	//设置点跟踪的上下文
	pCornerTrackContex->pSrcColImg = pTrackContext->pSrcColImg;
	pCornerTrackContex->preGrayImg = pTrackContext->preGrayImg;
	pCornerTrackContex->pCurGrayImg = pTrackContext->pGrayImg;
	pCornerTrackContex->pTrackRios = &pTrackContext->LastRioRecvec;
	pCornerTrackContex->tracktype = pTrackContext->TrackType;
	pCornerTrackContex->FilterType = pTrackContext->FilterType;
	

	//设置轨迹管理的上下文
	ptrajecyMangContex->pSrcColImg = pTrackContext->pSrcColImg;
	ptrajecyMangContex->pSrcGrayImg = pTrackContext->pGrayImg;
	ptrajecyMangContex->pLastRioRecs = &pTrackContext->LastRioRecvec;

}

/*********************
函数名：Process
输入参数：pTrackContext:跟踪上下文
输出：无
功能：去除角点与轨迹点靠近的点，并将轨迹的点击都添加到点集合中去

***********************/
void CTrack4Corner::AddTrajecyPoint(vector<Pointf> *pAddPoints)
{
	//	
	const  list<Ctracjecy*>* pUsedtrajecyvecs = m_pTrajecyManage->GetUsedTrajecy();

	list<Ctracjecy*>::const_iterator listIter;


	//将上一帧有效轨迹点加入到额外补充点中
	for (listIter = pUsedtrajecyvecs->begin(); listIter != pUsedtrajecyvecs->end(); listIter++)
	{
		const Ctracjecy *ptrajecy = *listIter;

		//目标轨迹的删除角点-》轨迹层处理，在形成目标层时会将一部分不在目标内的轨迹设置无效，但没有做删除工作！
		if (ptrajecy->m_bValid)
		{
			//轨迹最后一个点
			const Pointf* pLastPOint = ptrajecy->GetLastPoint();
			pAddPoints->push_back(*pLastPOint);
		}

	}


}

 list<Ctracjecy*>* CTrack4Corner::GetUsedTrajecy()
{
	return m_pTrajecyManage->GetUsedTrajecy();
}

void CTrack4Corner::Process(const  TrackRuntimeContext* pTrackContext)
{
	assert(m_pCornerFind && m_pCornerTrack);

	//
	if (!pTrackContext->preGrayImg || !pTrackContext->pGrayImg)
	{
		return;
	}

	//设置上下文
	CornerFindRuntimeContext CornerFidContex;
	CornertrackRuntimeContext CornerTrackContex;
	trajecyMangeRuntimeContext  trajecyMangContex;

	//配置上下文
	SetContext(pTrackContext, &CornerFidContex, &CornerTrackContex, &trajecyMangContex);


#ifdef SHOW_TIME
	double ts = cvGetTickCount();
#endif
	//step1:角点提取
	m_pCornerFind->FindCorner(&CornerFidContex);

	
#ifdef SHOW_TIME
	double dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "m_pCornerFind->FindCorner耗时" << dtaketime << endl;
	ts = cvGetTickCount();
#endif

	//step2:获得角点提取结果放入到角点匹配的上下文
	CornerTrackContex.pcorners = m_pCornerFind->GetCorner();
	
	//step3:进行前后2帧的角点跟踪
	m_pCornerTrack->CornerTrackProcess(&CornerTrackContex);


#ifdef SHOW_TIME
	dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "m_pCornerTrack->CornerTrackProcess耗时" << dtaketime << endl;
	ts = cvGetTickCount();
#endif

	//step3:将前后2帧匹配结果形成轨迹
	//备注：一点要求当前帧轨迹最后一点加入到下次提取角点上下文addpoints中去！
	trajecyMangContex.pTrackpairs = m_pCornerTrack->GetCornerTrackResult();

	//根据匹配结果，对轨迹进行新增加（角点提取的点），和更新（匹配上的），删除(匹配不上的)
	m_pTrajecyManage->process(&trajecyMangContex);
	


#ifdef SHOW_TIME
	dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "m_pTrajecyManage->process耗时" << dtaketime << endl;
#endif
	
	

}

const TrackPair*  CTrack4Corner::GetLandMarkTrackPair()
{
	const TrackPair* pLandkMatchPoints = m_pCornerTrack->GetLanMarkTrackResult();

	return pLandkMatchPoints;

}
