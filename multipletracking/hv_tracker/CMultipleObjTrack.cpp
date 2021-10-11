#include "CMultipleObjTrack.h"
#include "CTracker4Corner.h"
#include "Cobject.h"
#include "CObjgroup.h"
#include "opencv2/opencv.hpp"
#include <cmath>
#include <fstream>
#ifdef SAVERES
int g_SaveNum = 0;
#endif 


CMultipleObjTrack::CMultipleObjTrack():
m_pCornerTrack(NULL),
m_ObjGroup(NULL),
m_pMulTrackContext(NULL),
m_pCurdata(NULL),
m_predata(NULL),
m_uFramSeq(0),
m_fscale(1.0f)
{
	
	m_pMulTrackContext = new MutilTrackContext();


}


CMultipleObjTrack::~CMultipleObjTrack()
{
    if(m_pCornerTrack)
    {
        delete m_pCornerTrack;
        m_pCornerTrack = NULL;
    }
    if(m_ObjGroup)
    {
        delete m_ObjGroup;
        m_ObjGroup = NULL;
    }
    if(m_pMulTrackContext)
    {
        delete m_pMulTrackContext;
        m_pMulTrackContext = NULL;
    }
    if(m_pCurdata)
        delete [] m_pCurdata;
    m_pCurdata = NULL;
    if(m_predata)
        delete [] m_predata;
    m_predata = NULL;
    if(m_pTempdata)
        delete [] m_pTempdata;
    m_pTempdata = NULL;
}
void CMultipleObjTrack::MuptileTrackInit(int nMaxTrajecNum)
{
	if (!m_pCornerTrack)
	{
		m_pCornerTrack = new CTrack4Corner(nMaxTrajecNum);//主要通过2帧的匹配，获得轨迹信息
	}

	if (!m_ObjGroup)
	{

		m_ObjGroup = new CObjgroup(m_pCornerTrack->GetTraTrajecyManage()); //通过轨迹进行目标的预测
	}



}
void CMultipleObjTrack::SetobjectRuntimeContext(
	const MutilTrackContext *pMulTrackContext, \
	CobjectRuntimeContext *pTrackObjContext \
	)
{
	pTrackObjContext->pTrajecys = m_pCornerTrack->GetUsedTrajecy();
	pTrackObjContext->pObjRects = &pMulTrackContext->Blobs;
	pTrackObjContext->pSrcImg = pMulTrackContext->pSrcColImg;
	pTrackObjContext->pGrayImg = pMulTrackContext->pGrayImg;
	pTrackObjContext->dtimestampe = pMulTrackContext->dTimestamp;
	pTrackObjContext->bUniqueObj = pMulTrackContext->bUniqueObj;
	pTrackObjContext->uFramSeq = m_uFramSeq;
}

//单个检测目标创建
void CMultipleObjTrack::SingleTrackobjCreter(cv::Rect rect, float fConfidence,\
											cv::Rect &fuseRec,\
											vector<cv::Point>* pvLandMark)
{
	
	SetLandMark(pvLandMark);

#ifdef SHOW_IMG
	ShowDetAndtrackLandMark(pvLandMark);
#endif

	//将检测目标结果配置跟踪上下的blob属性
	m_pMulTrackContext->Blobs.clear();

	m_pMulTrackContext->bUniqueObj = true;

	cv::Rect DetRec = cv::Rect(int(rect.x / m_fscale), \
		int(rect.y / m_fscale), \
		int(rect.width / m_fscale),
		int(rect.height / m_fscale));
	DetRec &= cv::Rect(0, 0, m_pMulTrackContext->pGrayImg->cols,
		m_pMulTrackContext->pGrayImg->rows);
	m_pMulTrackContext->Blobs.push_back(Blob(DetRec, fConfidence));


	//设置目标跟踪熟悉
	CobjectRuntimeContext TrackObjContext;

	//设置上下文
	SetobjectRuntimeContext(m_pMulTrackContext, &TrackObjContext);

	m_ObjGroup->ObjCreaterandFuseTrack(&TrackObjContext);

#ifdef SHOW_IMG
	TrackAndDetFuseRes();
#endif

	GetfuseRec(rect, fConfidence,fuseRec);
}
void CMultipleObjTrack::SetLandMark(vector<cv::Point>* pvLandMark)
{
	m_vLandMark.clear();
	if (pvLandMark)
	{
		for (int n = 0; n < (int)pvLandMark->size(); n++)
		{
			m_vLandMark.push_back(cv::Point2d(\
				(*pvLandMark)[n].x / m_fscale, \
				(*pvLandMark)[n].y / m_fscale));
		}
	}
}
void CMultipleObjTrack::GetfuseRec(cv::Rect Detrect, float fConfidence,cv::Rect &fuseRec)
{
	fuseRec = Detrect;
	const list<Cobject*> UsedObjVec = GetMatureTrackObjs();//正在使用的轨迹
	if (!m_pMulTrackContext->bUniqueObj || UsedObjVec.empty())
	{
		return;
	}

	Cobject* pTrackObj = UsedObjVec.front();

	if ( pTrackObj->m_fConfidence > 0.5f ||\
	    (fConfidence < 0.3f && pTrackObj->m_fConfidence > fConfidence) )
	{		
		fuseRec.x = (int)(pTrackObj->m_rect.x * m_fscale);
		fuseRec.y = (int)(pTrackObj->m_rect.y * m_fscale);
		fuseRec.width = (int)(pTrackObj->m_rect.width *  m_fscale);
		fuseRec.height = (int)(pTrackObj->m_rect.height * m_fscale);
	}

}

//根据提供的目标创建新目标
void CMultipleObjTrack::MuptileTrackobjCreter(const vector<cv::Rect> &rectVector, \
	const vector<float> &fconf)
{
	//将检测目标结果配置跟踪上下的blob属性
	m_pMulTrackContext->Blobs.clear();

	m_pMulTrackContext->bUniqueObj = false;

	//
	for (int i = 0; i < (int)rectVector.size(); i++)
	{
		m_pMulTrackContext->Blobs.push_back(Blob(cv::Rect(int(rectVector[i].x / m_fscale), \
			int(rectVector[i].y / m_fscale), \
			int(rectVector[i].width / m_fscale),
			int(rectVector[i].height / m_fscale)), fconf[i])
			);
	}

	//设置目标跟踪熟悉
	CobjectRuntimeContext TrackObjContext;

	//设置上下文
	SetobjectRuntimeContext(m_pMulTrackContext, &TrackObjContext);

	m_ObjGroup->ObjCreaterandFuseTrack(&TrackObjContext);
}

cv::Rect2f CMultipleObjTrack::RestirctRect(cv::Rect2f Rect, cv::Size size)
{
	cv::Rect2f rect;
	rect.x = MIN(MAX(Rect.x, 0), size.width - 1);
	rect.y = MIN(MAX(Rect.y, 0), size.height - 1);

	rect.width = MIN(size.width - 1 - Rect.x, Rect.width);
	rect.height = MIN(size.height - 1 - Rect.y, Rect.height);

	return rect;
}
void CMultipleObjTrack::SaveTrackResult()
{
	list<Cobject*>::const_iterator Iter;
	//获得上一帧的目标信息
	const list<Cobject*>* pObjvec = m_ObjGroup->GetTrackObjs();

	ofstream Trackfile("trackRes.txt", ios::trunc | ios::out);

	if (Trackfile)
	{
		for (Iter = pObjvec->begin(); Iter != pObjvec->end(); Iter++)
		{
			const Cobject* pObj = *Iter;

			cv::Rect2f Track2Ori = cv::Rect2f(pObj->m_rect.x * m_fscale,
				pObj->m_rect.y * m_fscale, pObj->m_rect.width * m_fscale, pObj->m_rect.height * m_fscale);

			Trackfile << Track2Ori.x << "," << Track2Ori.y << "," \
				<< Track2Ori.width << "," << Track2Ori.height << ","\
				<< pObj->m_fConfidence << "," << pObj->m_dtimeWithoutFeedback << endl;
		}


		if (pObjvec->empty())
		{
			Trackfile << -1 << "," << -1 << "," << 0 << "," << 0 << endl;
		}

		Trackfile.close();
	}
	else
	{
		cout << " FILE trackRes.txt can not open" << endl;
	}
}

void CMultipleObjTrack::SetTrackRuntimeContext(TrackRuntimeContext* pTrackcontext, double dTsCur)
{
	pTrackcontext->preGrayImg = m_pMulTrackContext->preGrayImg; //上一帧图像
	pTrackcontext->pGrayImg = m_pMulTrackContext->pGrayImg;//当前帧图像
	pTrackcontext->pSrcColImg = m_pMulTrackContext->pSrcColImg;//彩色
	pTrackcontext->FilterType = m_pMulTrackContext->TrackPro.FilterType;//对2帧之间的过滤机制
	pTrackcontext->nMaxCornerNum = m_pMulTrackContext->TrackPro.nMaxCornerNum;
	pTrackcontext->nMaxTrajecyNum = m_pMulTrackContext->TrackPro.nMaxTrajecyNum;
	pTrackcontext->cornertype = m_pMulTrackContext->TrackPro.cornertype;//采取提取角点方式Goodfeature
	pTrackcontext->TrackType = m_pMulTrackContext->TrackPro.TrackType;//2帧间角点跟踪方式


#ifdef SHOW_IMG
	//针对无法给出彩色图像时，有需要调试观察窗口，则用灰度图组建三通道彩色图来代替
	if (!m_pMulTrackContext->pSrcColImg)
	{
		//强制丢弃const 属性
		MutilTrackContext *pContext = const_cast<MutilTrackContext *>(m_pMulTrackContext);

		//创建指针对象
		pContext->pSrcColImg = new cv::Mat(pTrackcontext->pGrayImg->rows, \
			pTrackcontext->pGrayImg->cols, CV_8UC3);

		//用灰度值组建三通道的伪彩色图像
		vector <cv::Mat> grayVec;
		grayVec.push_back(*pTrackcontext->pGrayImg);
		grayVec.push_back(*pTrackcontext->pGrayImg);
		grayVec.push_back(*pTrackcontext->pGrayImg);
		cv::merge(grayVec, *pContext->pSrcColImg);
		pTrackcontext->pSrcColImg = pContext->pSrcColImg;//彩色
	}
#endif



	//获得上一帧的目标信息
	const list<Cobject*>* pObjvec = m_ObjGroup->GetTrackObjs();

	list<Cobject*>::const_iterator Iter;

	for (Iter = pObjvec->begin(); Iter != pObjvec->end(); Iter++)
	{
		const Cobject* pObj = *Iter;

		//如果历史目标与当前目标时间相差大的,则不处理
		if (dTsCur - pObj->m_dupdatatime < TIME_OUT_THRESH)
		{
			pTrackcontext->LastRioRecvec.push_back(RestirctRect(pObj->m_rect, \
				cv::Size(pTrackcontext->pGrayImg->cols, pTrackcontext->pGrayImg->rows)));
		}
	}


	vector<cv::Point>::const_iterator pointIter = m_vLandMark.begin();

	for (; pointIter != m_vLandMark.end(); pointIter++)
	{
		pTrackcontext->AddPoints.push_back(Pointf((float)pointIter->x, (float)pointIter->y, -1, true));
	}

}

//多目标跟踪接口
void CMultipleObjTrack::MuptileTrackProcess(const cv::Mat *pImg,
	double dTsCur, bool bBigFaceScene, \
	bool bEvlTrack,
	CornerType cornertype, \
	CornerTrackType TrackType,
	int nMaxCornerNum,
	int nMaxTrajecyNum)
{

#ifdef SHOW_IMG
	//CloseAllShowWindow();
#ifdef SAVERES
	 g_SaveNum++;
#endif 
#endif

	//初始化类对象
	MuptileTrackInit(nMaxTrajecyNum);

	//配置跟踪上下文
	m_uFramSeq++;
	SetMulObjTrackcontext(pImg, dTsCur, bBigFaceScene, \
		cornertype, TrackType, nMaxCornerNum, nMaxTrajecyNum);


	TrackRuntimeContext Trackcontext;
	SetTrackRuntimeContext(&Trackcontext, dTsCur);

#ifdef SHOW_TIME
	double ts = cvGetTickCount();
#endif

	//同过前后2帧以角点形式形成轨迹，此时出来的轨迹都是有效的！
	m_pCornerTrack->Process(&Trackcontext); //6
	

#ifdef SHOW_TIME
	double dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "m_pCornerTrack->Process耗时" << dtaketime<<endl;
#endif

	//设置目标跟踪属性
	CobjectRuntimeContext TrackObjContext;

	//设置上下文
	SetobjectRuntimeContext(m_pMulTrackContext, &TrackObjContext);

#ifdef SHOW_TIME
	ts = cvGetTickCount();
#endif

	//对轨迹进行归类目标进行处理
	m_ObjGroup->ObjTrackProcess(&TrackObjContext);

#ifdef SHOW_TIME
	dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "m_ObjGroup->ObjProcess耗时" << dtaketime << endl;
#endif

	//是否用文本记录当前跟踪结果，用于跟踪性能评估
	if (bEvlTrack)
	{
		SaveTrackResult();
	}
}

const list<Cobject*>*CMultipleObjTrack::GetTrackObjs()
{
	return m_ObjGroup->GetTrackObjs();
}
vector<Pointf> CMultipleObjTrack::GetLandMarkTrack()
{
	vector<Pointf> ptVec;

	const TrackPair* plandTrackpair = m_pCornerTrack->GetLandMarkTrackPair();

	for (int i = 0; i < (int)plandTrackpair->CornerMatchRes.size(); i++)
	{
		Pointf LandTrackPt = plandTrackpair->CornerMatchRes.at(i).MatchPoints;
		LandTrackPt.x *= m_fscale;
		LandTrackPt.y *= m_fscale;
		LandTrackPt.bLandMark = plandTrackpair->CornerMatchRes.at(i).status;
		ptVec.push_back(LandTrackPt);
	}

	return  ptVec;

}
const list<Cobject*> CMultipleObjTrack::GetMatureTrackObjs()
{
	list<Cobject*> CobjListVec;

	const list<Cobject*>* pCUsedObjVec = m_ObjGroup->GetTrackObjs();//正在使用的轨迹

	list<Cobject*>::const_iterator iter = pCUsedObjVec->begin();
	for (; iter != pCUsedObjVec->end(); iter++)
	{
		if ((*iter)->m_bMature)
		{
			CobjListVec.push_back((*iter));
		}
	}

	return CobjListVec;
}

void CMultipleObjTrack::SetMulObjTrackcontext(const cv::Mat *pImg, \
	double dTsCur, bool bBigFaceScene, CornerType cornertype, \
	CornerTrackType TrackType, int nMaxCornerNum,
	int nMaxTrajecyNum)
{
	
	
	if (!pImg)
	{
		return;
	}



	//前后帧切换
	if (!m_CurGray.empty())
	{
		//m_pCurGray.copyTo(m_preGray);
		memcpy(m_preGray.data, m_CurGray.data, m_CurGray.step * m_CurGray.rows);
	}


	int nw = pImg->cols;
	int nh = pImg->rows;

	if (!m_pCurdata)
	{
		const int size = pImg->cols * pImg->rows;
		m_pCurdata = new uchar[size];
		m_predata = new uchar[size];
		m_pTempdata = new uchar[size];
	}
	
	//big face
	if (bBigFaceScene)
	{
		m_fscale = 4;

		if (m_CurGray.empty())
		{
			m_CurGray = cv::Mat(int(nh / m_fscale), int(nw / m_fscale), CV_8UC1, m_pCurdata);
		}

		if (pImg->channels() == 3)
		{
			cv::Mat Gray = cv::Mat(pImg->rows, pImg->cols, CV_8UC1, m_pTempdata);
            cv::cvtColor(*pImg, Gray, cv::COLOR_BGR2BGRA);
			cv::resize(Gray, m_CurGray, cv::Size(m_CurGray.cols, m_CurGray.rows), cv::INTER_NEAREST);
		}
		else
		{
			cv::resize(*pImg, m_CurGray, cv::Size(m_CurGray.cols, m_CurGray.rows), cv::INTER_NEAREST);
		}

	}
	else
	{
		m_fscale = 1;

		if (m_CurGray.empty())
		{
			m_CurGray = cv::Mat(nh, nw, CV_8UC1, m_pCurdata);
		}


		if (pImg->channels() == 3)
		{
            cv::cvtColor(*pImg, m_CurGray, cv::COLOR_BGR2GRAY);
		}
		else
		{
			memcpy(m_CurGray.data, pImg->data, pImg->step * pImg->rows);
		}

	}

	if (m_preGray.empty())
	{
		m_preGray = cv::Mat(m_CurGray.rows, m_CurGray.cols, CV_8UC1, m_predata);
		m_CurGray.copyTo(m_preGray);
	}


	//单位秒，精确到毫秒
	double dtimeStampe = dTsCur;

	m_pMulTrackContext->preGrayImg = &m_preGray; //上一帧图像
	m_pMulTrackContext->pGrayImg = &m_CurGray;//当前帧图像

	m_pMulTrackContext->dTimestamp = dtimeStampe;

	m_pMulTrackContext->pSrcColImg = (pImg->channels() == 3) ? pImg : NULL;//彩色
	m_pMulTrackContext->TrackPro.FilterType = vector<CornerTrackFilterType>{};//对2帧之间的过滤机制
	m_pMulTrackContext->TrackPro.cornertype = cornertype;//采取提取角点方式Goodfeature，Fast
	m_pMulTrackContext->TrackPro.TrackType = TrackType; //2帧间角点跟踪方式
	m_pMulTrackContext->TrackPro.nMaxCornerNum = nMaxCornerNum;
	m_pMulTrackContext->TrackPro.nMaxTrajecyNum = nMaxTrajecyNum;

	//将检测目标结果配置跟踪上下的blob属性
	m_pMulTrackContext->Blobs.clear();
}

#ifdef SHOW_IMG
void CMultipleObjTrack::CloseAllShowWindow()
{
	cv::destroyWindow("Track_Final_TD");
	cv::destroyWindow("LandTrack");
	cv::destroyWindow("Track_Res");
	cv::destroyWindow("join Pose_predit Trajecys");
	cv::destroyWindow("corner_Match");
	cv::destroyWindow("combineImg");
	cv::destroyWindow("轨迹");
}
void CMultipleObjTrack::TrackAndDetFuseRes()
{
	cv::Mat colImg;
	cv::resize(*m_pMulTrackContext->pSrcColImg, colImg, cv::Size(m_pMulTrackContext->pGrayImg->cols,
		m_pMulTrackContext->pGrayImg->rows));

	for (int i = 0; i < m_pMulTrackContext->Blobs.size(); i++)
	{
		const Blob* pblob = &m_pMulTrackContext->Blobs.at(i);
		cv::rectangle(colImg, pblob->rec, CV_RGB(0, 255, 0), 3);

		string str = to_string(pblob->fConf);
		cv::putText(colImg, str, cv::Point(pblob->rec.x, MAX(0, pblob->rec.y - 10))
			, 1, 1, CV_RGB(0, 255, 0));
	}
	const list<Cobject*>* puseObj = m_ObjGroup->GetTrackObjs();
	list<Cobject*>::const_iterator usedIter = puseObj->begin();

	for (; usedIter != puseObj->end(); usedIter++)
	{
		Cobject* pObject = *usedIter;
		cv::rectangle(colImg, cv::Rect(pObject->m_rect.x, pObject->m_rect.y, pObject->m_rect.width,
			pObject->m_rect.height), CV_RGB(255, 0, 0));

		char cdata[256];
		sprintf_s(cdata, "cof%0.2f,TNF:%0.2f,M:%d", pObject->m_fConfidence, pObject->m_dtimeWithoutFeedback,\
			pObject->m_bMature);
		cv::putText(colImg, cdata, cv::Point(pObject->m_rect.x, \
			MAX(0, pObject->m_rect.y + pObject->m_rect.height + 10))
			, 1, 1, CV_RGB(255, 0, 0));

	}


	cv::imshow("Track_Final_TD", colImg);
#ifdef SAVERES
	cv::imwrite(to_string(g_SaveNum%15) + ".jpg", colImg);
#endif
}

void CMultipleObjTrack::ShowDetAndtrackLandMark(vector<cv::Point>* pvLandMark)
{
	if (!pvLandMark)
	{
		return;
	}

	if (m_pMulTrackContext->pSrcColImg)
	{
		//原始图
		cv::Mat Img = m_pMulTrackContext->pSrcColImg->clone();
		vector<Pointf> pts = GetLandMarkTrack();
		float fscale = (m_pMulTrackContext->pGrayImg->cols == m_pMulTrackContext->pSrcColImg->cols)
			? m_fscale : 1;
		for (int n = 0; n < pvLandMark->size(); n++)
		{
			cv::circle(Img, cv::Point(int((*pvLandMark)[n].x / fscale), int((*pvLandMark)[n].y / fscale)), 5, CV_RGB(0, 255, 0));

			if (!pts.empty())
			{
				if (pts[n].bLandMark)
				{
					cv::circle(Img, cv::Point(int(pts[n].x / fscale), int(pts[n].y / fscale)), 3, CV_RGB(255, 0, 0));
					cv::line(Img, cv::Point(int((*pvLandMark)[n].x / fscale), int((*pvLandMark)[n].y / fscale)), \
						cv::Point(int(pts[n].x / fscale), int(pts[n].y / fscale)), CV_RGB(255, 255, 0), 1);
				}

			}

		}

		cv::imshow("LandTrack", Img);
		cv::waitKey(10);
	}
}
#endif
