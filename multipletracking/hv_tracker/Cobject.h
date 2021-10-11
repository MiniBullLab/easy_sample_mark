#ifndef COBJECT_H
#define COBJECT_H

#include "utility.h"

class Ctracjecy;
class CTrajecyManage;
typedef struct DetRec
{
	cv::Rect2f Rec;
	double dtime;
	DetRec(cv::Rect2f r, double t)
	{
		Rec = r;
		dtime = t;
	}
}DetRec;

class Cobject
{
public:
	Cobject();

	~Cobject();

public:
	void Clear(CTrajecyManage *pTrajecyManage);
	void DeloldRecbyDet(double dtimestampe);

	//将目标中的轨迹都设置为人脸检测激活
	void SetTrackjecysActivedByDet();

	//由检测框激活的轨迹额数
	int GetTrajceyNumActivedbyDet(); 

public:

	vector<Ctracjecy*> m_Trajecys;//目标所拥有的轨迹

	cv::Rect2f m_rect; //定义目标的位置都在提取轨迹图像系中

	list<Pointf> m_CenTrajecy; //中心轨迹

	int m_nObjID;//目标对象的ID，每个目标会被分配一个ID

	double m_dtimeWithoutFeedback;//间隔多少时间没有反馈（单位秒）

	double m_dFeedbackByDetTime; //上一帧被更新到的时间

	float m_fVaryscale; //目标尺度变换

	vector<DetRec> m_DetRecVec;//跟踪时被检测反馈的信息

	int m_SeriunsafeNum;//当检测框与跟踪框冲突（IOU低于阈值）的连续次数

	float  m_fConfidence;//置信度

	int m_boundTrunc_W; //边界截断W
	int m_boundTrunc_H; //边界截断W

	unsigned int m_lastUnsafeFrame; //上一次检测与跟踪框不一致的帧数

	bool m_bMature; //成熟状态

	double m_dupdatatime; //更新时间




};




#endif
