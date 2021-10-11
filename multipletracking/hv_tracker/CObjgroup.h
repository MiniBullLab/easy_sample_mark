#ifndef COBJGROUP_H
#define COBJGROUP_H
#include "utility.h"

class Ctracjecy;
class Cobject;
class CTrajecyManage;

#define MAX_OBJ_NUM  16
#define MAX_OBJ_CENTRAJECY_LENTH 8  //最多记录目标最近的8帧的中心轨迹

#define TRAJECY_RELIABLE_THRESH 4 //如果目标在参与预测时有缓冲机制！必须运动矢量与先前可靠的保持一定相似度！
#define PI 3.1415926
#define low_Filter_Factor 0.01
#define  fDetSameTrackThresh  0.3
#define  fDetSafeTrackThresh  0.9
#define  fTrackSafeTrackThresh  0.8

typedef struct CobjectRuntimeContext
{
	const cv::Mat* pSrcImg;
	const cv::Mat* pGrayImg;
	list<Ctracjecy *>* pTrajecys;//使用的当前真有效目标轨迹
	const vector<Blob>* pObjRects;//当前帧检测到的目标
	double dtimestampe;
	unsigned int uFramSeq;
	bool bUniqueObj; //目标唯一性:仅仅跟踪一个目标
	CobjectRuntimeContext()
	{
		pSrcImg = NULL;
		pGrayImg = NULL;
		pTrajecys = NULL;
		pObjRects = NULL;
		dtimestampe = 0;
		uFramSeq = 0;
		bUniqueObj = false;
	}
}CobjectRuntimeContext;

class  CObjgroup
{
public:
	CObjgroup(CTrajecyManage *pTrajecyManage);
	~CObjgroup();

public:

	//目标跟踪预测接口
	void ObjTrackProcess(CobjectRuntimeContext* pCObjContext);

	const list<Cobject*>* GetTrackObjs(){ return &m_UsedObjVec; }

	void ObjCreaterandFuseTrack(CobjectRuntimeContext* pCObjContext);


private:

	//产生new的目标
	void  GenerateNewObj(CobjectRuntimeContext* pCObjContext);

	//更新目标所属的轨迹
	void AssignNewTrajecysToObj( list<Ctracjecy *>* pTrajecys);

	//根据轨迹预测目标的位置
	void PreditObjPosbyTrajects(double dtimestampe);

	//计算匹配点之间的位移量
	MatchFactor CalScaleAndPosMov(const vector< Ctracjecy *>* pTrajecys, Cobject* pObj);

	//预测跟踪框与检测框的结果融合
	void PreidtfuseDetRec(const vector<Blob>* pObjRects, double dtimestampe = 0);

	//重置轨迹的目标归属
	void ResetTrackAssign(list<Ctracjecy *>* pTrajecys);

	vector< Ctracjecy *> FilterByMainMove(vector< Ctracjecy *>* pObjTrajecys);

	//删除目标
	void DelObj(double dtime, bool bDelAllOb = false);

	bool IsDetRec2ObjOk(const Blob *pblob,Cobject* pTrackObj);

	//设置目标置信度
	void SetTrackobjConfidence();

private:
	Cobject* m_pobjs; //跟踪的目标对象
	list<Cobject*> m_UsedObjVec;//正在使用的轨迹
	list<Cobject *> m_FreeObjVec; //空闲的目标
	CobjectRuntimeContext* m_pObjRuntimeContext;
	CTrajecyManage *m_pTrajecyManage;
private:

	//获得当前可用的目标
	Cobject* applyFreeObj(); //申请空闲的目标地址空间，作为新目标使用！

	int m_ObjID; //目标ID（针对所有）

	int m_FreamSeq; //当前帧号

#ifdef SHOW_TRAJECY_IMG
	void ShowObjJoinPosepreditTrajecy(const vector< Ctracjecy *>* pobjtrajecys,MatchFactor* pres =NULL);
#endif

	
};
#endif

