#include "CObjgroup.h"
#include "opencv2/opencv.hpp"
#include "CTrajecy.h"
#include "Cobject.h"
#include "CTrajecyManage.h"

#ifdef SAVERES
extern int g_SaveNum;
#endif

CObjgroup::CObjgroup(CTrajecyManage *pTrajecyManage) :
m_pobjs(NULL),
m_ObjID(-1),
m_FreamSeq(0)
{
	m_pTrajecyManage = pTrajecyManage;

	m_pobjs = new Cobject[MAX_OBJ_NUM];

	for (int i = 0; i < MAX_OBJ_NUM; i++)
	{
		//m_pobjs[i].nObjID = -1; 

		//初始化时默认所有目标都是空闲状态
		m_FreeObjVec.push_back(m_pobjs + i);

	}


}

CObjgroup::~CObjgroup()
{
    if(m_pobjs)
        delete [] m_pobjs;
    m_pobjs = NULL;
}

void CObjgroup::ObjCreaterandFuseTrack(CobjectRuntimeContext* pCObjContext)
{
	m_pObjRuntimeContext = pCObjContext;

	//检测框和预测框融合处理
	PreidtfuseDetRec(pCObjContext->pObjRects, pCObjContext->dtimestampe);


	//根据目标最终确认的位置，重新分配当前帧轨迹的目标归属问题
	ResetTrackAssign(pCObjContext->pTrajecys);

	//删除目标
	DelObj(pCObjContext->dtimestampe);

	//产生新目标
	GenerateNewObj(pCObjContext);

}

void CObjgroup::ObjTrackProcess(CobjectRuntimeContext* pCObjContext)
{
	m_pObjRuntimeContext = pCObjContext;


	//将轨迹层未分配目标的轨迹分配目标（一般指当前帧新增的轨迹时进行一次目标ID的分配）
	//输入当前轨迹，更新轨迹的m_nObjID状态
	AssignNewTrajecysToObj(pCObjContext->pTrajecys);

	

	//根据属于目标的轨迹进行目标位置的预测
	PreditObjPosbyTrajects(pCObjContext->dtimestampe);

	

	//根据目标最终确认的位置，重新分配当前帧轨迹的目标归属问题
	ResetTrackAssign(pCObjContext->pTrajecys);



	//删除目标
	DelObj(pCObjContext->dtimestampe);


	//设置跟踪目标置信度
	SetTrackobjConfidence();


#ifdef SHOW_IMG
	//当前跟踪结果
	list<Cobject*>::iterator usedIter = m_UsedObjVec.begin();
	cv::Mat colImg;
	cv::resize(*pCObjContext->pSrcImg, colImg, cv::Size(pCObjContext->pGrayImg->cols,
	pCObjContext->pGrayImg->rows));

	for (; usedIter != m_UsedObjVec.end(); usedIter++)
	{
		Cobject* pObject = *usedIter;
		cv::rectangle(colImg, cv::Rect(pObject->m_rect.x, pObject->m_rect.y, pObject->m_rect.width,
			pObject->m_rect.height), CV_RGB(255, 0, 0));

		char cdata[256];
		sprintf_s(cdata, "cof%0.2f,TNF:%0.2f,M:%d", pObject->m_fConfidence, pObject->m_dtimeWithoutFeedback,
			pObject->m_bMature);
		cv::putText(colImg, cdata, cv::Point(pObject->m_rect.x, \
			MAX(0, pObject->m_rect.y + pObject->m_rect.height + 10))
			, 1, 1, CV_RGB(255, 0, 0));

	}

	
	cv::imshow("Track_Res", colImg);
#ifdef SAVERES
	cv::imwrite(to_string(g_SaveNum%15) + ".jpg", colImg);
#endif

#endif

	m_FreamSeq++;

}

void CObjgroup::SetTrackobjConfidence()
{

	//先清空目标所拥有的轨迹
	list<Cobject*>::iterator usedIter = m_UsedObjVec.begin();
	for (; usedIter != m_UsedObjVec.end(); usedIter++)
	{
		Cobject* pObj = *usedIter;

		//2时，轨迹都是从人脸检测框真实提供的（默认在检测框提供的角点形成的轨迹是可靠的）
		if (2 == pObj->m_CenTrajecy.size())
		{
			
			//设置当前的目标内的轨迹为人脸框激活的轨迹。
			pObj->SetTrackjecysActivedByDet();

			//当匹配轨迹个数>=12,置信度为1
			pObj->m_fConfidence = MIN(1.0f, pObj->m_Trajecys.size() / 12.0f);
		}
		else
		{
			//统计由人脸检测提供的轨迹点个数/12，归一化到[0,1]
			pObj->m_fConfidence = MIN(1.0f, pObj->GetTrajceyNumActivedbyDet() / 12.0f);
		}

	}
}

//产生new的目标
void CObjgroup::GenerateNewObj(CobjectRuntimeContext* pCObjContext)
{
	list<Cobject*>::iterator usedIter;
	if (!pCObjContext->pObjRects)
	{
		return;
	}

	const int nSeriosThesh = 2; //连续检测与track冲突阈值

	//遍历输入的检测框目标
	for (int i = 0; i < (int)pCObjContext->pObjRects->size(); i++)
	{
		//认为新目标规则：目前暂时认为新目标与历史没有重叠！！
		bool bNewObj = true;

		const cv::Rect* pInputRec = &pCObjContext->pObjRects->at(i).rec;
		float fInputScore = pCObjContext->pObjRects->at(i).fConf;
		//遍历历史跟踪的目标中，确认当前检测目标是否为新增的目标
		for (usedIter = m_UsedObjVec.begin(); usedIter != m_UsedObjVec.end(); usedIter++)
		{
			Cobject *pObj = *usedIter;

			cv::Rect objRec((int)pObj->m_rect.x, (int)pObj->m_rect.y, (int)pObj->m_rect.width, (int)pObj->m_rect.height);

			cv::Rect interSectionRec = objRec & (*pInputRec);
			cv::Rect UnionRec = objRec | (*pInputRec);

			//跟踪目标与检测目标占比
			float  fratio = (float)(interSectionRec.area()) / UnionRec.area();

			//只要该检测目标与已有的目标有交集，且大与一定比例，则认为此目标不是新目标
			if (fratio > fDetSameTrackThresh)
			{
				bNewObj = false;
				break;
			}
		}

		
		//如果确认是新的目标
		if (bNewObj)
		{
			//如果目标唯一性（仅跟踪一个目标），且当前历史跟踪目标不为空，则清空历史跟踪目标
			//(认为历史目标是虚假目标)
			if (pCObjContext->bUniqueObj && !m_UsedObjVec.empty())
			{
			
				//有时候单个目标跟踪时，检测会出现错误的检测，造成创建伪目标（一般误检测到小目标），为了避免此情况
				//对该创建的目标要多次确认才删除历史跟踪目标，创建新目标
				Cobject*pObj = *m_UsedObjVec.begin();

				if (pObj->m_fConfidence > 0.85f) //如果目标置信度比较，此时忽略检测
				{
					continue;
				}

				pObj->m_SeriunsafeNum++;
				
				//来确认是否是连续认为不可靠
				if (pObj->m_SeriunsafeNum > 1 && pCObjContext->uFramSeq != \
					pObj->m_lastUnsafeFrame + TRACK_INTERVAL_FRAME)
				{
					pObj->m_SeriunsafeNum = 0;
				}
				
				pObj->m_lastUnsafeFrame = pCObjContext->uFramSeq;

				//连续说跟踪目标出错或者大目标，则删除该目标
				if ( pObj->m_SeriunsafeNum > nSeriosThesh  
					|| pInputRec->area() >  pObj->m_rect.area()* 1.5f//此条件是由于跟踪结束，检测误检到了小脸，后有检测正常
					|| fInputScore > 0.99)
				{
					DelObj(pCObjContext->dtimestampe,true);
				}
				else
				{
					continue;
				}	
			}

			Cobject *pNewObj = applyFreeObj();

			//如果申请到新目标空间
			if (pNewObj)
			{				
#ifdef CHECK_TRACK_RUN
				CHECK_THROWCRASH(-1 != pNewObj->m_nObjID || !pNewObj->m_CenTrajecy.empty(),
					新目标初始化状态异常)
#endif
				//设置目标{ID，位置，中心}
				pNewObj->m_nObjID = (++m_ObjID);

				pNewObj->m_dFeedbackByDetTime = pCObjContext->dtimestampe;
				pNewObj->m_dtimeWithoutFeedback = 0;

				cv::Rect rect = (*pInputRec) & cv::Rect(0, 0, pCObjContext->pGrayImg->cols,
					                                    pCObjContext->pGrayImg->rows);

				pNewObj->m_rect = cv::Rect2f((float)rect.x, (float)rect.y, (float)rect.width, (float)rect.height);
				pNewObj->m_CenTrajecy.push_back(Pointf(pInputRec->x + pInputRec->width / 2.0f, \
					                                   pInputRec->y + pInputRec->height / 2.0f));

			     //如果检测置信度高，则认为目标成熟
				if (fInputScore > 0.9f)
				{
					pNewObj->m_bMature = true;
					pNewObj->m_fConfidence = 1;
				}
				
				pNewObj->m_dupdatatime = pCObjContext->dtimestampe;

			}
		}
	}
}

//更新目标所属的轨迹(当轨迹长度=2时，此时不知道新产生的轨迹属于哪个目标不知道，所以要确认属于哪个目标)
void CObjgroup::AssignNewTrajecysToObj(list<Ctracjecy *>* pTrajecys)
{
	//观察当前轨迹属于哪个目标
	list<Ctracjecy *>::iterator iter;
	list<Cobject*>::iterator usedIter;
	for (iter = pTrajecys->begin(); iter != pTrajecys->end(); iter++)
	{
		//当前轨迹
		Ctracjecy * pTrajecy = *iter;

		//取倒数第二个点（备注：当前m_UsedObjVec目标的位置还是上一帧的位置！）
		const Pointf* pLastSedPoint = pTrajecy->GetLastNPoint(2);

		if (!pLastSedPoint)
		{


#ifdef CHECK_TRACK_RUN
			CHECK_THROWCRASH(0=== pLastSedPoint, 轨迹没有获得点);
#endif
			pTrajecy->m_bValid = false;
			continue;
		}

		//将没有归属目标的轨迹进行目标归属的处理
		if (-1 == pTrajecy->m_nObjID)
		{
#ifdef CHECK_TRACK_RUN
			CHECK_THROWCRASH(2 != pTrajecy->m_nTrackLength, 不是新增的轨迹没有分配目标);
#endif
			Cobject* pBelongtoObj = NULL;
			int nMaxY = 0; //如果目标重叠，轨迹归属目标Y靠下的

			//遍历所有用的目标，观察其轨迹
			for (usedIter = m_UsedObjVec.begin(); usedIter != m_UsedObjVec.end(); usedIter++)
			{
				//此时的目标位置还没更新，还是上一帧的位置信息
				Cobject* pObj = *usedIter;

				int nRight = int(pObj->m_rect.x + pObj->m_rect.width);
				int nBottom = int(pObj->m_rect.y + pObj->m_rect.height);

				//观察轨迹点是否落在该目标矩形框内
				if (pLastSedPoint->x >= pObj->m_rect.x &&
					pLastSedPoint->y >= pObj->m_rect.y
					&&  pLastSedPoint->x <= nRight
					&&pLastSedPoint->y <= nBottom)
				{

					if (nBottom > nMaxY)
					{
						nMaxY = nBottom;
						pBelongtoObj = pObj;
						pTrajecy->m_nObjID = pObj->m_nObjID;
					}
				}
			}

			//此时认为该轨迹属于该目标！
			if (pBelongtoObj)
			{
				pBelongtoObj->m_Trajecys.push_back(pTrajecy);
			}
			else
			{
				pTrajecy->m_bValid = false;
			}
		}
	}


}
//根据当前帧的最终目标确定的位置来确定轨迹归属问题
//如果轨迹同时落在2个或者以上的目标区域，最后归属给框底边最下面的那个
void CObjgroup::ResetTrackAssign(list<Ctracjecy *>* pTrajecys)
{
	//先清空目标所拥有的轨迹
	list<Cobject*>::iterator usedIter = m_UsedObjVec.begin();
	for (; usedIter != m_UsedObjVec.end(); usedIter++)
	{
		(*usedIter)->m_Trajecys.clear();
	}


	//遍历所有轨迹
	list<Ctracjecy *>::iterator iter;
	for (iter = pTrajecys->begin(); iter != pTrajecys->end(); iter++)
	{
		//
		Ctracjecy * pTrajecy = *iter;

		//如果该轨迹不属于任何目标，则返回
		if (-1 == pTrajecy->m_nObjID)
		{
			pTrajecy->m_bValid = false;
			continue;
		}

		//获得当前轨迹最后一帧的位置
		const Pointf* pLastPoint = pTrajecy->GetLastPoint();

		if (!pLastPoint)
		{
			pTrajecy->m_bValid = false;
			continue;
		}

		//将轨迹归属哪个目标
		Cobject* pBelongtoObj = NULL;
		int nMaxY = 0; //如果目标重叠，轨迹归属目标Y靠下的


		//遍历所有用的目标
		for (usedIter = m_UsedObjVec.begin(); usedIter != m_UsedObjVec.end(); usedIter++)
		{

			//此时的目标位置还没更新，还是上一帧的位置信息
			Cobject* pObj = *usedIter;

			int nRight = int(pObj->m_rect.x + pObj->m_rect.width);
			int nBottom = int(pObj->m_rect.y + pObj->m_rect.height);

			//观察轨迹点是否落在该目标矩形框内
			if (pLastPoint->x >= pObj->m_rect.x + pObj->m_rect.width * SHRINKRATIO &&
				pLastPoint->y >= pObj->m_rect.y + pObj->m_rect.height * SHRINKRATIO &&
				pLastPoint->x <= (nRight - pObj->m_rect.width * SHRINKRATIO) &&
				pLastPoint->y <= (nBottom - pObj->m_rect.height * SHRINKRATIO))
			{

				if (nBottom > nMaxY)
				{
					nMaxY = nBottom;
					pBelongtoObj = pObj;
					pTrajecy->m_nObjID = pObj->m_nObjID;
				}
			}
		}

		//如果轨迹归属目标ID与当前目标ID不一致，则该目标不考虑
		if (pBelongtoObj && pTrajecy->m_nObjID != pBelongtoObj->m_nObjID)
		{
			pBelongtoObj = NULL;
		}

		//找到轨迹所属目标
		if (pBelongtoObj)
		{
			pBelongtoObj->m_Trajecys.push_back(pTrajecy);
		}
		else
		{
			//如果轨迹不归属任何目标，则将目标设置为无效，轨迹层会将其删除掉
			pTrajecy->m_bValid = false;
		}

	}

	//删除应目标没有用的设置valid =false轨迹
	m_pTrajecyManage->DelTrajectoys();
}

vector< Ctracjecy*> CObjgroup::FilterByMainMove(vector< Ctracjecy *>* pObjTrajecys)
{

	//遍历每个轨迹
	vector<float> dxNVec;
	vector<float> dyNVec;

	vector<float> dx2Vec;
	vector<float> dy2Vec;
	//遍历当前所有轨迹中长度大于TRAJECY_RELIABLE_THRESH，作为其统计主方向
	for (int i = 0; i < (int)pObjTrajecys->size(); i++)
	{
		Ctracjecy * pTrajecy = pObjTrajecys->at(i);

		//轨迹长度>TRAJECY_RELIABLE_THRESH认为是可靠轨迹
		if (pTrajecy->m_nTrackLength > TRAJECY_RELIABLE_THRESH)
		{
			const Pointf* pLastPoint = pTrajecy->GetLastPoint();
			const Pointf* pLast_N_Point = pTrajecy->GetLastNPoint(TRAJECY_RELIABLE_THRESH);
			const Pointf* pLast_2_Point = pTrajecy->GetLastNPoint(2);
			float fdx = pLastPoint->x - pLast_N_Point->x;
			float fdy = pLastPoint->y - pLast_N_Point->y;
			dxNVec.push_back(fdx);
			dyNVec.push_back(fdy);
			dx2Vec.push_back(pLastPoint->x - pLast_2_Point->x);
			dy2Vec.push_back(pLastPoint->y - pLast_2_Point->y);
		}
	}

	if (dxNVec.empty())
	{
		return  *pObjTrajecys;
	}
	else
	{
		//如果目标已经确立了主方向，最对轨迹不满足主方向的短轨迹则暂时不参与目标预测的计算
		//避免背景的干扰
		vector< Ctracjecy*> VaidTrajecys; //有效的轨迹
		Pointf MainMov; //轨迹主矢量
		sort(dxNVec.begin(), dxNVec.end());
		sort(dyNVec.begin(), dyNVec.end());
		float nMaxMovDis = 0.0;
		for (int i = 0; i < (int)dx2Vec.size(); i++)
		{
			nMaxMovDis = MAX(abs(dx2Vec[i]) + abs(dy2Vec[i]), nMaxMovDis);
		}

		float fMidx = dxNVec.at(dxNVec.size() / 2);
		float fMidy = dyNVec.at(dyNVec.size() / 2);

		MainMov.x = fMidx;
		MainMov.y = fMidy;

		//计算运动最大的
		for (int i = 0; i < (int)pObjTrajecys->size(); i++)
		{
			Ctracjecy * pTrajecy = pObjTrajecys->at(i);
			const Pointf* pLastPoint = pTrajecy->GetLastPoint();
			const Pointf* pLast_2_Point = pTrajecy->GetLastNPoint(2);

			bool bMeetDis = abs(pLastPoint->x - pLast_2_Point->x) + \
				abs(pLastPoint->y - pLast_2_Point->y) > 0.3 * nMaxMovDis;

			if (!bMeetDis)
			{
				if (pTrajecy->m_nTrackLength == TRAJECY_RELIABLE_THRESH)
				{
					pTrajecy->m_bValid = false;
				}

				continue;
			}
			else if (pTrajecy->m_nTrackLength < TRAJECY_RELIABLE_THRESH)
			{
				VaidTrajecys.push_back(pTrajecy);
				continue;
			}

			//轨迹长度>TRAJECY_RELIABLE_THRESH认为是可靠轨迹
			if (pTrajecy->m_nTrackLength == TRAJECY_RELIABLE_THRESH)
			{
				const Pointf* pLastPoint = pTrajecy->GetLastPoint();
				const Pointf* pLast_N_Point = pTrajecy->GetLastNPoint(TRAJECY_RELIABLE_THRESH);

				Pointf Mov; //轨迹主矢量
				Mov.x = pLastPoint->x - pLast_N_Point->x;
				Mov.y = pLastPoint->y - pLast_N_Point->y;

				float fAmplitude = sqrtf(Mov.x *Mov.x + Mov.y * Mov.y);
				float fMidAmplitude = sqrtf(MainMov.x *MainMov.x + MainMov.y * MainMov.y);
				float coSimlary = 0;

				if (fAmplitude < 1e-5 || fMidAmplitude < 1e-5)
				{
					coSimlary = fabs(fMidAmplitude - fAmplitude) > 2 ? -1.0f : 1.0f;
				}
				else
				{
					coSimlary = (Mov.x * MainMov.x + Mov.y * MainMov.y) / (fAmplitude *fMidAmplitude);
				}

				if (coSimlary > 0.5f)
				{
					VaidTrajecys.push_back(pTrajecy);
				}
				else
				{
					pTrajecy->m_bValid = false;
				}

			}
			else if (pTrajecy->m_nTrackLength > TRAJECY_RELIABLE_THRESH)
			{
				VaidTrajecys.push_back(pTrajecy);
			}
		}

		return VaidTrajecys;
	}
}

//显示参与目标的轨迹
#ifdef SHOW_TRAJECY_IMG
void CObjgroup::ShowObjJoinPosepreditTrajecy(const vector< Ctracjecy *>* pobjtrajecys, MatchFactor* pres)
{
	//
	cv::Mat ColImg = m_pObjRuntimeContext->pSrcImg->clone();

	cv::resize(ColImg, ColImg, cv::Size(m_pObjRuntimeContext->pGrayImg->cols, \
		m_pObjRuntimeContext->pGrayImg->rows));

	for (int i = 0; i < pobjtrajecys->size(); i++)
	{
		Ctracjecy * ptracjecy = pobjtrajecys->at(i);
		ptracjecy->DrawTrajecy(&ColImg);
	}

	if (pres)
	{
		std::string std;
		std = "Scale:" + to_string(pres->fscale);
		cv::putText(ColImg, std, cv::Point(150, 150),1.5,1.5, CV_RGB(255, 0, 0));
	}

	cv::imshow("join Pose_predit Trajecys", ColImg);

}
#endif

//根据轨迹预测目标的位置
void  CObjgroup::PreditObjPosbyTrajects(double dtimestampe)
{
	//遍历所有用的目标，观察其轨迹，并计算其偏移量
	list<Cobject*>::iterator usedIter;
	for (usedIter = m_UsedObjVec.begin(); usedIter != m_UsedObjVec.end(); usedIter++)
	{
		Cobject* pObj = *usedIter;

		//此时目标所含的轨迹包含2部分{新增轨迹和以前标记属于该目标的轨迹}
		//注意：此时历史标注的轨迹可能在轨迹层删除掉了
		//所以此时包含的轨迹不一定都归属该目标，还必须check下m_nObjID

		//提取真实有效的轨迹,要剔除被删除的轨迹
		vector< Ctracjecy *> objtrajecys;
		if (pObj->m_Trajecys.empty())
		{
			continue;
		}

		vector<Ctracjecy*>::iterator TrajecysIter = pObj->m_Trajecys.begin();
		for (; TrajecysIter != pObj->m_Trajecys.end();)
		{
			Ctracjecy * pTrajecy = *TrajecysIter;

			//表明此轨迹是不属于该目标的
			if (pTrajecy->m_nObjID != pObj->m_nObjID)
			{
				TrajecysIter = pObj->m_Trajecys.erase(TrajecysIter);
			}
			else
			{
				TrajecysIter++;
			}
		}

		//根据轨迹预测目标的移动量
		MatchFactor res = CalScaleAndPosMov(&pObj->m_Trajecys, pObj);

		//显示
#ifdef SHOW_TRAJECY_IMG
		ShowObjJoinPosepreditTrajecy(&pObj->m_Trajecys, &res);
#endif
		if (res.bSucss)
		{
#ifdef PRINTF_INF
			cout << "scale" << res.fscale << endl;
			cout << "fx,fy" << res.fx << "," << res.fy << endl;
#endif
			//获得真实目标的预测位置
			float fs1 = 0.5f * (res.fscale - 1.0f) * pObj->m_rect.width;
			float fs2 = 0.5f * (res.fscale - 1.0f) * pObj->m_rect.height;

			cv::Rect_<float> objRec;
			objRec.x = (pObj->m_rect.x + res.fx - fs1);
			objRec.y = (pObj->m_rect.y + res.fy - fs2);
			objRec.width = (pObj->m_rect.width * res.fscale);
			objRec.height = (pObj->m_rect.height * res.fscale);

			cv::Size TrackObjSize((int)objRec.width, (int)objRec.height);
			objRec = objRec & cv::Rect_<float>(0, 0, (float)m_pObjRuntimeContext->pGrayImg->cols,
				(float)m_pObjRuntimeContext->pGrayImg->rows);

			//如果目标太靠近边界，尺寸变截断。
			pObj->m_boundTrunc_W += (int)(TrackObjSize.width - objRec.width);
			pObj->m_boundTrunc_H += (int)(TrackObjSize.height - objRec.height);
			

			pObj->m_rect.x = objRec.x;
			pObj->m_rect.y = objRec.y;
			pObj->m_rect.width = objRec.width;
			pObj->m_rect.height = objRec.height;
			pObj->m_dupdatatime = dtimestampe;	
		}

		//获得多少时间没有被检测反馈
		pObj->m_dtimeWithoutFeedback = dtimestampe - pObj->m_dFeedbackByDetTime;

		if (MAX_OBJ_CENTRAJECY_LENTH == pObj->m_CenTrajecy.size())
		{
			//删除最早那个点，即第一个点
			list<Pointf>::iterator iter = pObj->m_CenTrajecy.begin();
			pObj->m_CenTrajecy.erase(iter);
			pObj->m_CenTrajecy.push_back(Pointf(pObj->m_rect.x + pObj->m_rect.width / 2
				, pObj->m_rect.y + pObj->m_rect.height / 2));
		}
		else
		{
			pObj->m_CenTrajecy.push_back(Pointf(pObj->m_rect.x + pObj->m_rect.width / 2
				, pObj->m_rect.y + pObj->m_rect.height / 2));
		}
		
	}

}


//计算匹配点之间的位移量
MatchFactor  CObjgroup::CalScaleAndPosMov(const vector<Ctracjecy *>* pTrajecys, Cobject* pObj)
{
	MatchFactor MatchRes;

	if (pTrajecys->empty())
	{
		MatchRes.bSucss = false;
		return MatchRes;
	}
	vector<const Pointf*> inputs, Matchs;
	const  int nMatchunNum = int(pTrajecys->size());
	inputs.reserve(nMatchunNum);
	Matchs.reserve(nMatchunNum);



	//遍历每个轨迹 ,取出轨迹最后2个点信息
	for (int i = 0; i < nMatchunNum; i++)
	{
		const Ctracjecy* ptyjecy = pTrajecys->at(i);
		inputs.push_back(ptyjecy->GetLastNPoint(2));
		Matchs.push_back(ptyjecy->GetLastNPoint(1));
	}


	//两两之间计算尺度
	vector<float> fscaleVec, fdxVec, fdyVec;
	for (int i = 0; i < nMatchunNum; i++)
	{
		for (int j = 0; j < nMatchunNum; j++)
		{
			if (j != i)
			{
				float dst1 = (inputs.at(i)->x - inputs.at(j)->x) *
					(inputs.at(i)->x - inputs.at(j)->x) + (inputs.at(i)->y - inputs.at(j)->y) *
					(inputs.at(i)->y - inputs.at(j)->y);

				//对应匹配点之间的距离
				float dst2 = (Matchs.at(i)->x - Matchs.at(j)->x) *
					(Matchs.at(i)->x - Matchs.at(j)->x) + (Matchs.at(i)->y - Matchs.at(j)->y) *
					(Matchs.at(i)->y - Matchs.at(j)->y);

				float fscale = dst1 > 1e-3 ? sqrtf((dst2 / dst1)) : 1.0f;

				fscaleVec.push_back(fscale);
			}
		}
	}

	//计算点与匹配点之间的位移量
	for (int i = 0; i < nMatchunNum; i++)
	{
		float fdx = (Matchs.at(i)->x - inputs.at(i)->x);
		float fdy = (Matchs.at(i)->y - inputs.at(i)->y);

		fdxVec.push_back(fdx);
		fdyVec.push_back(fdy);
	}


	if (fscaleVec.size() > 3)
	{
		sort(fscaleVec.begin(), fscaleVec.end());
		MatchRes.fscale = fscaleVec[fscaleVec.size() / 2];
	}

	if (!fdxVec.empty())
	{
		sort(fdxVec.begin(), fdxVec.end());
		sort(fdyVec.begin(), fdyVec.end());
		MatchRes.fx = fdxVec[fdxVec.size() / 2];
		MatchRes.fy = fdyVec[fdyVec.size() / 2];
	}

	pObj->m_fVaryscale = float((1 - low_Filter_Factor)*pObj->m_fVaryscale + low_Filter_Factor* MatchRes.fscale);
	
	MatchRes.fscale = pObj->m_fVaryscale;
	MatchRes.bSucss = true;
	return MatchRes;
}
Cobject*  CObjgroup::applyFreeObj()
{
	//是否存在空闲的轨迹
	if (m_FreeObjVec.empty())
	{
		return  NULL;
	}
	else
	{
		//使用最后一个
		Cobject* pObj = m_FreeObjVec.back();
		m_FreeObjVec.pop_back();

		//放入到使用的轨迹当中去
		m_UsedObjVec.push_back(pObj);

		return  pObj;
	}
}


//检查检测目标在历史目标的合理性
bool CObjgroup::IsDetRec2ObjOk(const Blob *pblob,Cobject* pTrackObj)
{
	cv::Rect2f Detrec = pblob->rec;
	
	//如果检测队列中数目大于一定数目，如果当前检测的面积 > 历史的中值*0.85,则认为当前的检测可靠，否则为不可靠
	//如果数目较小时，必须满足当前检测的面积 > 当前跟踪面积*0.85，则认为当前可靠，否则为不可靠
	vector<int> nAreaVec;
	vector<DetRec>::iterator iter;
	for (iter = pTrackObj->m_DetRecVec.begin(); iter != pTrackObj->m_DetRecVec.end(); iter++)
	{
		nAreaVec.push_back(int(iter->Rec.area()));
	}

	//跟踪目标附属的检测目标个数少于阈值，不能进行合理性判别.
	const int nMinJugeNum = 5;
	const float fAreaRatioThresh = 0.85f;
	if (pTrackObj->m_DetRecVec.size() < nMinJugeNum)
	{
		//检测目标比跟踪目标大,默认检测合理
		if (Detrec.area() > pTrackObj->m_rect.area() * fAreaRatioThresh &&\
			pblob->fConf > 0.8)
		{
			return true;
		}
		else 
		{   
			/*Det < Track 时，必须持续检测到小才使用！(宁可跟踪区域大的也不采纳检测小的
			  避免因为检测噪声，发生后续一些列无法检测的问题)*/
			return false;
		}
		
	}

	sort(nAreaVec.begin(), nAreaVec.end());

	int nMidDetArea = nAreaVec[nAreaVec.size() / 2];

	//规定在dtimeThresh内，统计得到中值人脸面积，当前检测的的区域不能低于中值人脸的阈值
	if (Detrec.area() > fAreaRatioThresh * nMidDetArea 
		&& (Detrec.area() * fAreaRatioThresh < nMidDetArea || pblob->fConf > 0.8))
	{
		return true;
	}
	else
	{
 		return false;
	}



}
//预测跟踪框与检测框的结果融合
void CObjgroup::PreidtfuseDetRec(const vector<Blob>* pObjRects, double dtimestampe)
{

	list<Cobject*>::iterator usedIter;
	

	//遍历历史跟踪的目标中，确认当前检测目标是否为新增的目标
	for (usedIter = m_UsedObjVec.begin(); usedIter != m_UsedObjVec.end(); usedIter++)
	{
		Cobject *pTrackObj = *usedIter;

		float fMaxratio = -1;

		const Blob* pBestMatchDet = NULL;

		//遍历输入的检测框目标,是否有重叠度超过30%
		for (int i = 0; i < (int)pObjRects->size(); i++)
		{
			const cv::Rect* pInputRec = &pObjRects->at(i).rec;
			cv::Rect objRec((int)pTrackObj->m_rect.x, (int)pTrackObj->m_rect.y, (int)pTrackObj->m_rect.width, (int)pTrackObj->m_rect.height);
			cv::Rect interSectionRec = objRec & (*pInputRec);
			cv::Rect UnionRec = objRec | (*pInputRec);

			//跟踪与检测交集/检测目标占比
			float  fratio = (float)(interSectionRec.area()) / UnionRec.area();

			if (fratio > fMaxratio)
			{
				pBestMatchDet = &pObjRects->at(i);
				fMaxratio = fratio;
			}
		}
			
		//判断检测目标合理性，避免检测错误引起的问题
		//检测目标与跟踪目标重叠，使用检测目标结果
		if (fMaxratio >= fDetSameTrackThresh)
		{
			pTrackObj->DeloldRecbyDet(dtimestampe);


			bool bUseDetRec = pBestMatchDet->fConf > 0.95f ||
							  fMaxratio > fDetSafeTrackThresh ||
							  IsDetRec2ObjOk(pBestMatchDet, pTrackObj);

			if (bUseDetRec)
			{

#ifdef PRINTF_INF
				std::cout << "IOU:" << fMaxratio << ",检测置信度:" << pBestMatchDet->fConf << endl;
				std::cout << "目标被检测框重置！！" << endl;
#endif
				if (fMaxratio < fTrackSafeTrackThresh)
				{
					pTrackObj->m_rect = pBestMatchDet->rec;
					pTrackObj->m_fVaryscale = 1;
					pTrackObj->m_dtimeWithoutFeedback = 0;
					pTrackObj->m_CenTrajecy.back().x = pBestMatchDet->rec.x + pBestMatchDet->rec.width / 2.0f;
					pTrackObj->m_CenTrajecy.back().y = pBestMatchDet->rec.y + pBestMatchDet->rec.height / 2.0f;
					
					pTrackObj->m_boundTrunc_W = 0;
					pTrackObj->m_boundTrunc_H = 0;
				}
			
				//设置当前的目标内的轨迹为人脸框激活的轨迹。
				pTrackObj->SetTrackjecysActivedByDet();

				//跟踪被检测反馈次数大于阈值或当前检测反馈的置信度比较高
				pTrackObj->m_bMature = true;
			}


			pTrackObj->m_dFeedbackByDetTime = dtimestampe;

			if (fMaxratio > fTrackSafeTrackThresh || pBestMatchDet->fConf > 0.85f)
			{
				pTrackObj->m_DetRecVec.push_back(DetRec(pBestMatchDet->rec, dtimestampe));

			}
			
		}
	}

}


//删除目标
void CObjgroup::DelObj(double dtime,bool bDelAllOb)
{
	list<Cobject*>::iterator iter;

	if (bDelAllOb)
	{
		for (iter = m_UsedObjVec.begin(); iter != m_UsedObjVec.end();)
		{
			Cobject* pobj = (*iter);
			pobj->Clear(m_pTrajecyManage);
			m_FreeObjVec.push_back(pobj);
			iter = m_UsedObjVec.erase(iter);	
		}

		return;
	}

	
	int nMinObjTrajecyNum = 3;
	int nBorderThresh = 15;

	//如果目标轨迹里没有有效轨迹
	for (iter = m_UsedObjVec.begin(); iter != m_UsedObjVec.end();)
	{
		Cobject* pobj = (*iter);
		
		//目标轨迹太少，刚创建目标时轨迹为0
		bool bLittleTrajecys = ( (int)pobj->m_Trajecys.size() < nMinObjTrajecyNum
			                    && 2 != pobj->m_CenTrajecy.size());
		
		//目标太小
		bool bSmall =  MIN(pobj->m_rect.width, pobj->m_rect.height) < 10;
		
		//目标靠近边界
		bool bNearBored = pobj->m_rect.x < nBorderThresh || pobj->m_rect.y < nBorderThresh
			|| (pobj->m_rect.x + pobj->m_rect.width) > m_pObjRuntimeContext->pGrayImg->cols - nBorderThresh
			|| (pobj->m_rect.y + pobj->m_rect.height) > m_pObjRuntimeContext->pGrayImg->rows - nBorderThresh;
		
		//目标检测反馈超时（很久没有检测目标反馈）
		float fTimeoutThesh = pobj->m_fConfidence > 0.75 ? 10.0f :5.0f;
		if (pobj->m_fConfidence < 0.4)
		{
			fTimeoutThesh = 1;
		}

		bool bDetFedTimeOut = pobj->m_dtimeWithoutFeedback > fTimeoutThesh;//5秒检测器没有反馈

		bool bTrackTimeout = dtime - pobj->m_dupdatatime > TIME_OUT_THRESH; //目标跟踪超时了
		
		if ( bLittleTrajecys || bSmall \
			|| (bNearBored && pobj->m_dtimeWithoutFeedback > 2.0f\
					&& pobj->m_fConfidence < 0.6f)  \
			|| MAX(pobj->m_boundTrunc_W, pobj->m_boundTrunc_H) > 10\
			|| bDetFedTimeOut || bTrackTimeout  )
		{
			
			pobj->Clear(m_pTrajecyManage);
			m_FreeObjVec.push_back(pobj);
			iter = m_UsedObjVec.erase(iter);	
		}
		else
		{
			iter++;
		}	
	}
}
