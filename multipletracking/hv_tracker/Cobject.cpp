#include "Cobject.h"
#include "CTrajecy.h"
#include "CTrajecyManage.h"

Cobject::Cobject()
:m_nObjID(-1),
m_dtimeWithoutFeedback(0),
m_dFeedbackByDetTime(0),
m_fVaryscale(1),
m_SeriunsafeNum(0),
m_fConfidence(0.0f),
m_boundTrunc_W(0),
m_boundTrunc_H(0),
m_bMature(false)
{


}

Cobject::~Cobject()
{
}

void Cobject::DeloldRecbyDet(double dtimestampe)
{
	//只保留最近时间阈值范围内检测反馈结果
	const double dtimeThresh = 1.5;
	vector<DetRec>::iterator iter;

	for (iter = m_DetRecVec.begin(); iter != m_DetRecVec.end();)
	{
		if (dtimestampe - iter->dtime > dtimeThresh)
		{
			iter = m_DetRecVec.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void Cobject::Clear(CTrajecyManage *pTrajecyManage)
{
	vector<Ctracjecy*>::iterator iter;

	for (iter = m_Trajecys.begin(); iter != m_Trajecys.end(); iter++)
	{
		Ctracjecy* ptrajecy = *iter;

		//设置轨迹无效，轨迹的回收由TrajecyManage管理
		ptrajecy->m_bValid = false;

	}

	//这边立即回收无效的轨迹，避免下次申请新的轨迹失败。
	pTrajecyManage->DelTrajectoys();

	m_Trajecys.clear();

	m_CenTrajecy.clear(); //中心轨迹

	m_nObjID = -1;//目标对象的ID，每个目标会被分配一个ID

	m_fVaryscale = 1;

	m_SeriunsafeNum = 0;
	m_lastUnsafeFrame = 0;
	m_bMature = false;

	m_dFeedbackByDetTime = 0;
	m_dtimeWithoutFeedback = 0;
	m_fConfidence = 0;
	m_boundTrunc_H = 0;
	m_boundTrunc_W = 0;
	m_DetRecVec.clear();

}

//将目标内的所有轨迹都设置为检测激活状态
void Cobject::SetTrackjecysActivedByDet()
{
	vector<Ctracjecy*>::iterator iter;

	for (iter = m_Trajecys.begin(); iter != m_Trajecys.end(); iter++)
	{
		Ctracjecy* ptrajecy = *iter;
		ptrajecy->m_bActivedByDet = true;
	}

}

//返回由检测框激活的轨迹个数
int Cobject::GetTrajceyNumActivedbyDet()
{
	vector<Ctracjecy*>::iterator iter;

	int nNum = 0;
	for (iter = m_Trajecys.begin(); iter != m_Trajecys.end(); iter++)
	{
		Ctracjecy* ptrajecy = *iter;

		if (ptrajecy->m_bActivedByDet)
		{
			nNum++;
		}
	}

	return  nNum;

}