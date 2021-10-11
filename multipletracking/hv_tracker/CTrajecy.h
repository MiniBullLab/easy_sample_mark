#ifndef CTRJECY_H
#define CTRJECY_H
#include "utility.h"

#define MAX_TRAJECY_LENGTH 16 //轨迹最多记录的长度 必须是2的次数幂

class  Ctracjecy
{
public:
	Ctracjecy();
	~Ctracjecy();

public:
	Pointf* m_Points;
	int m_nTrakMiss; //如果轨迹当前帧未匹配到，即次数+1
	bool m_bValid; //轨迹有效
	int m_nId; //自身轨迹编号
	int m_nObjID; // 这条轨迹属于哪个每个
	int m_nTrackLength;
	int m_naddressId; //地址Id,初始化后就固定了
	unsigned char m_col[3];
	bool m_bActivedByDet;// 该轨迹是有检测框激活的，而不是跟踪过程中增加的

public:

	void clearTrajecy();

	//判断轨迹是否有效
	bool CheckTrajecyIsValid();

	//将轨迹设置为无效轨迹
	void SetTrackUnvalid();

	const Pointf* GetLastPoint()const;

	const Pointf* GetLastNPoint(int n = 1)const;

	void DrawTrajecy(cv::Mat *pImg, float fScale = 1, int nDrawTrackMaxlen = MAX_TRAJECY_LENGTH)const ;

	
};
#endif

