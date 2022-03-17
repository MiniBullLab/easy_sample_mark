#include "CTrajecy.h"
#include "opencv2/opencv.hpp"


Ctracjecy::Ctracjecy():
m_Points(NULL)
{
	m_nTrakMiss = 0;
	m_nId = -1;
	m_nTrackLength = 0;
	m_Points = new Pointf[MAX_TRAJECY_LENGTH];
	m_bValid = false;
	m_nObjID = -1;
	m_bActivedByDet = false;

	for (int i = 0; i < 3; i++)
	{
		m_col[i] = rand() % 255;
	}
}

Ctracjecy::~Ctracjecy()
{
    if(m_Points)
        delete [] m_Points;
    m_Points = NULL;
}

void Ctracjecy::clearTrajecy()
{
	m_nTrakMiss = 0;
	m_nId = -1;
	m_bValid = false;
	m_nObjID = -1;
	m_bActivedByDet = false;

	//清空认为其中的点不在任何轨迹中
	for (int i = 0; i < min(MAX_TRAJECY_LENGTH, m_nTrackLength); i++)
	{
		m_Points[i].naddressId = -1;
	}

	m_nTrackLength = 0;
	
}

//判断轨迹是否有效
bool Ctracjecy::CheckTrajecyIsValid()
{
	//如果该轨迹匹配一次不成功
	//为无效轨迹，需要删除
	m_bValid = m_nTrakMiss  ? false : m_bValid;

	for (int j = 0; j < MIN(m_nTrackLength, MAX_TRAJECY_LENGTH); j++)
	{
		if (m_Points[j].naddressId != m_naddressId)
		{

#ifdef CHECK_TRACK_RUN
			CHECK_THROWCRASH(true, ID 不匹配);
#endif
			m_bValid = true;
			break;
		}
	}
		
	return  m_bValid;

}

void Ctracjecy::SetTrackUnvalid()
{
	m_bValid = false;
}

//获得最后一个点位置
const Pointf* Ctracjecy::GetLastPoint()const
{
	if (0 == m_nTrackLength)
	{
		return NULL;
	}

	int nLastIndex = (m_nTrackLength - 1) % MAX_TRAJECY_LENGTH;


	if (nLastIndex < 0 || nLastIndex >= MAX_TRAJECY_LENGTH)
	{
#ifdef CHECK_TRACK_RUN
		CHECK_THROWCRASH((nLastNIndex < 0 || nLastNIndex >= MAX_TRAJECY_LENGTH), nLastNIndex越界);
#endif
		return NULL;
	}

	return (m_Points + nLastIndex);

}

//获得倒数第N个点
const Pointf* Ctracjecy::GetLastNPoint(int n)const
{
	if (n >= MAX_TRAJECY_LENGTH)
	{
		return NULL;
	}

	int nLastNIndex = (m_nTrackLength - n) % MAX_TRAJECY_LENGTH;



	if (nLastNIndex < 0 || nLastNIndex >= MAX_TRAJECY_LENGTH)
	{

#ifdef CHECK_TRACK_RUN
		CHECK_THROWCRASH((nLastNIndex < 0 || nLastNIndex >= MAX_TRAJECY_LENGTH), nLastNIndex越界);
#endif
		return NULL;
	}
	return (m_Points + nLastNIndex);

}
void Ctracjecy::DrawTrajecy(cv::Mat *pImg, float fScale, int nDrawTrackMaxlen)const 
{
	cv::Scalar col = cv::Scalar(m_col[0], m_col[1], m_col[2]);

	if (m_nTrackLength <= MAX_TRAJECY_LENGTH)
	{
		for (int i = m_nTrackLength - 1; i > max(0,m_nTrackLength -  nDrawTrackMaxlen) + 1; i--)
		{
			cv::circle(*pImg, cv::Point(int(m_Points[i].x * fScale), int(m_Points[i].y *fScale)), 2, col);
			cv::line(*pImg, cv::Point(int(m_Points[i].x * fScale), int(m_Points[i].y *fScale)), \
			cv::Point(int(m_Points[i - 1].x*fScale), int(m_Points[i - 1].y*fScale)), col,  1);
		}
	}
	else
	{
		for (int i = m_nTrackLength - 1; i > m_nTrackLength - min(MAX_TRAJECY_LENGTH, nDrawTrackMaxlen) + 1; i--)
		{
			int nIndex = (i) % MAX_TRAJECY_LENGTH;
			int nSecdIndex = (i-1) % MAX_TRAJECY_LENGTH;

			cv::circle(*pImg, cv::Point(int(m_Points[nIndex].x * fScale), int(m_Points[nIndex].y *fScale)), 1, col);
			cv::line(*pImg, cv::Point(int(m_Points[nIndex].x * fScale), int(m_Points[nIndex].y* fScale)), \
				cv::Point(int(m_Points[nSecdIndex].x * fScale), int(m_Points[nSecdIndex].y* fScale)), \
				col, 1);
		}

	}


	
}
