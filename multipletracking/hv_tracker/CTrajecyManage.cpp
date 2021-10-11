#include "CTrajecyManage.h"
#include "opencv2/opencv.hpp"


#ifdef SHOW_IMG
typedef struct TrackViaslIf
{
	bool bVisual;
	cv::Scalar col;
	cv::Point textPoin;
	std::string str;
	cv::Rect   rec;
};
map<int, TrackViaslIf> g_trackMap;
vector<int>  g_WathcTrackID; //目前可见的轨迹ID
float g_fscale;
bool b_gFulltrackVisual = false; //所有轨迹都观察
cv::Rect g_TrackLenRec;
cv::Mat g_debugIMg;
cv::Mat g_debugIMg4mouseMessage; //在鼠标相应之前的图像
#define CV_IS_PT_IN_ROI(p, r)   (p.x >= r.x && p.y >= r.y && p.x < r.x + r.width \
	&& p.y < r.y + r.height)
const list<Ctracjecy *>* g_Usedtracjecy;
int g_VilsuMaxLenth = MAX_TRAJECY_LENGTH;
void OnMouse4VisualTrackChoose(
	int event,
	int x,
	int y,
	int flags,
	void* param
	)
{
	cv::Point ptMouse = cv::Point(x, y);
	cv::Mat  img = g_debugIMg4mouseMessage.clone();
	vector<int>::iterator IDiter;

	cv::Rect rSUb(g_TrackLenRec.x, g_TrackLenRec.y, \
		g_TrackLenRec.width / 3, g_TrackLenRec.height);

	cv::Rect rADD(g_TrackLenRec.x + g_TrackLenRec.width*2/3, g_TrackLenRec.y, \
		g_TrackLenRec.width / 3, g_TrackLenRec.height);

	switch (event)
	{
		case CV_EVENT_LBUTTONDOWN:	//left click to choose option
		{
			map<int, TrackViaslIf>::iterator iter = g_trackMap.begin();
			b_gFulltrackVisual =  false;
			
			//观察轨迹是否启动可见
			for (; iter != g_trackMap.end(); iter++)
			{
				cv::Rect r = iter->second.rec;

				if (CV_IS_PT_IN_ROI(ptMouse, r))
				{
					if (iter->second.bVisual)
					{
						img(r).setTo(cv::Scalar(128, 128, 128));
						iter->second.bVisual = false;
						IDiter = find(g_WathcTrackID.begin(), g_WathcTrackID.end(), iter->first);
						IDiter = g_WathcTrackID.erase(IDiter);
					}
					else
					{
						img(r).setTo(cv::Scalar(255, 255, 255));
						iter->second.bVisual = true;	
						g_WathcTrackID.push_back(iter->first);
					}

					img(cv::Rect(r.x, r.y, r.width, 5)).setTo(cv::Scalar(0, 0, 0));

					cv::putText(img, iter->second.str,
						iter->second.textPoin, 1, 1, iter->second.col);
				}

			}


			//+-操作显示	
			if (CV_IS_PT_IN_ROI(ptMouse, rSUb))
			{
				--g_VilsuMaxLenth;
				g_VilsuMaxLenth = max(0, g_VilsuMaxLenth);
			}

			if (CV_IS_PT_IN_ROI(ptMouse, rADD))
			{
				++g_VilsuMaxLenth;
				g_VilsuMaxLenth = min(MAX_TRAJECY_LENGTH, g_VilsuMaxLenth);
			}

			img(g_TrackLenRec).setTo(cv::Scalar(255, 255, 255));
			img(cv::Rect(g_TrackLenRec.x + g_TrackLenRec.width / 3, g_TrackLenRec.y,
				5, g_TrackLenRec.height)).setTo(cv::Scalar(0, 0, 0));
			img(cv::Rect(g_TrackLenRec.x + g_TrackLenRec.width * 2 / 3, g_TrackLenRec.y,
				5, g_TrackLenRec.height)).setTo(cv::Scalar(0, 0, 0));
			img(cv::Rect(g_TrackLenRec.x , g_TrackLenRec.y,
				g_TrackLenRec.width, 5)).setTo(cv::Scalar(0, 0, 0));

			cv::putText(img, "-", cv::Point(g_TrackLenRec.x, g_TrackLenRec.y + 25), 1.5
				, 1.5, cv::Scalar(0, 0, 255), 2);

			cv::putText(img, "+", cv::Point(g_TrackLenRec.x + g_TrackLenRec.width * 2 / 3 + 5
				, g_TrackLenRec.y + 25), 1.5
				, 1.5, cv::Scalar(0, 0, 255), 2);
			std::string std_MaxLeng = to_string(g_VilsuMaxLenth);
			cv::putText(img, std_MaxLeng, \
				cv::Point(g_TrackLenRec.x + g_TrackLenRec.width * 1 / 3 + 5
				, g_TrackLenRec.y + 25), 1.5
				, 1.5, cv::Scalar(0, 0, 255), 2);
			//img(cv::Rect(r.x, r.y, r.width, 5)).setTo(cv::Scalar(0, 0, 0));

			g_debugIMg4mouseMessage = img.clone();

			//遍历所有轨迹，并进行绘制
			list<Ctracjecy *>::const_iterator list_iter = g_Usedtracjecy->begin();
			for (; list_iter != g_Usedtracjecy->end(); list_iter++)
			{
				const Ctracjecy * ptyjecy = *list_iter;

				IDiter = find(g_WathcTrackID.begin(), g_WathcTrackID.end(), ptyjecy->m_nId);

				//只显示可见轨迹
				if (IDiter != g_WathcTrackID.end())
				{
					ptyjecy->DrawTrajecy(&img, g_fscale, g_VilsuMaxLenth);
				}

			}
			cv::imshow("轨迹", img);

			

			break;
		}
		case CV_EVENT_LBUTTONDBLCLK:  //left double click to change show model
		{

			break;
		}
		case CV_EVENT_RBUTTONDOWN:  //left double click to change show model
		{
			b_gFulltrackVisual = true;
			break;
		}
		case CV_EVENT_MOUSEMOVE:
		{

			break;
		}
		default:
		  break;
	 }
 
}
#endif

CTrajecyManage::CTrajecyManage(int nMaxTrajecNum) :
m_pTrackjecyPool(NULL),
m_pImg(NULL),
m_uCurId(0),
m_nMaxTrajNum(nMaxTrajecNum)
{
	//分配轨迹个数，以及每个轨迹的允许存储轨迹长度
	m_pTrackjecyPool = new Ctracjecy[nMaxTrajecNum];
	
	for (int i = 0; i < nMaxTrajecNum; i++)
	{
		//记录每个轨迹在数组中的位置
		m_pTrackjecyPool[i].m_naddressId = i; //存储地址编号索引

		//初始化时默认所有轨迹都是空闲状态
		m_Freetracjecy.push_back(m_pTrackjecyPool + i);

	}
}
CTrajecyManage::~CTrajecyManage()
{
    if(m_pTrackjecyPool)
        delete [] m_pTrackjecyPool;
    m_pTrackjecyPool = NULL;
}

void CTrajecyManage::process(const trajecyMangeRuntimeContext* pTrackMagContext)
{

#ifdef SHOW_TIME
	double ts = cvGetTickCount();
#endif
	//如果为空，说明没任何信息输入。
	if (pTrackMagContext->pTrackpairs->CornerMatchRes.empty())
	{
		//删除轨迹
		DelTrajectoys(true);
		return;
	}
	
#ifdef SHOW_TIME
	double dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "DelTrajectoys耗时" << dtaketime << endl;
	ts = cvGetTickCount();
#endif

	//调试时开启，如检查上一帧的轨迹在当前帧都参与了匹配（匹配或者匹配不上）
#ifdef CHECK_TRACK_RUN
	CheckTrajecyMatchPair(pTrackMagContext->pTrackpairs);
#endif

	//更新轨迹
	updataTrajectoy(pTrackMagContext->pTrackpairs);
	

#ifdef SHOW_TIME
	dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "updataTrajectoy耗时" << dtaketime << endl;
	ts = cvGetTickCount();
#endif

	//删除轨迹
	DelTrajectoys();


#ifdef SHOW_TIME
	dtaketime = (cvGetTickCount() - ts) / (cvGetTickFrequency()*1000.0f);
	cout << "___DelTrajectoys__耗时" << dtaketime << endl;
	ts = cvGetTickCount();
#endif

#ifdef DrawTrajecys
	//绘制轨迹,fscale:是将角点坐标系放到pImg图像坐标系的尺度
	float fscale = (float)pTrackMagContext->pSrcColImg->cols / pTrackMagContext->pSrcGrayImg->cols;
	DrawTrajecys(pTrackMagContext->pSrcColImg, fscale);
#endif

	
}
#ifdef SHOW_TRAJECY_IMG
void CTrajecyManage::DrawTrajecys(const cv::Mat *pImg, float fscale)const 
{
	/**********************获得绘制的图像******************/
	cv::Mat Img = pImg->clone();
	const int imgRows = 400;
	int nTrackBlobHeight = 40;
	int nTrackBlobWidth = 100;
	int nROwBlobNums = imgRows / nTrackBlobHeight;
	float fshinkScope = (float)Img.rows / imgRows; //避免图像尺寸太大，显示时缩小一定比例

	if (fabs(fshinkScope - 1.0f) > 1e-5)
	{
		//确保rows = 400;
		cv::resize(Img, Img, cv::Size(Img.cols / fshinkScope, imgRows));
		fscale /= fshinkScope;
	}

	//绘制轨迹状态一栏（灰色表示不可见，白色可见

	int nPadding = nTrackBlobWidth * cvCeil((float)(m_Usedtracjecy.size() + 1) / nROwBlobNums);
	g_debugIMg = cv::Mat(Img.rows, Img.cols + nPadding, CV_8UC3, cv::Scalar(0, 0, 0));
	Img.copyTo(g_debugIMg(cv::Rect(0, 0, Img.cols, Img.rows)));

	vector<int>::iterator iter;
	iter = g_WathcTrackID.begin();
	list<Ctracjecy*>::const_iterator list_iter;
	for (; iter != g_WathcTrackID.end();)
	{
		bool bFInd = false;
		list_iter = m_Usedtracjecy.begin();
		for (; list_iter != m_Usedtracjecy.end(); list_iter++)
		{
			const Ctracjecy * ptyjecy = *list_iter;
			if (ptyjecy->m_nId == *iter)
			{
				bFInd = true;
			}
		}

		if (bFInd)
		{
			iter++;
		}
		else
		{
			iter = g_WathcTrackID.erase(iter);
		}

	}
	/**********************绘制可视化轨迹******************/
	//当观察的轨迹数目为0，则自动将有效的所有轨迹都视为观察轨迹
	if (g_WathcTrackID.empty() || b_gFulltrackVisual)
	{
		g_WathcTrackID.clear();
		list<Ctracjecy *>::const_iterator list_iter = m_Usedtracjecy.begin();
		for (; list_iter !=  m_Usedtracjecy.end(); list_iter++)
		{
			const Ctracjecy * ptyjecy = *list_iter;
			g_WathcTrackID.push_back(ptyjecy->m_nId);
		}
	}


	cv::Rect rect(Img.cols, 0, nTrackBlobWidth, nTrackBlobHeight);
	int oFFNum = 0;
	g_trackMap.clear();
	cv::Point TextPoint(Img.cols + nTrackBlobWidth / 6, 25);

	list_iter = m_Usedtracjecy.begin();
	for (; list_iter!= m_Usedtracjecy.end(); list_iter++)
	{
		Ctracjecy * ptyjecy = *list_iter;
		iter = find(g_WathcTrackID.begin(), g_WathcTrackID.end(), ptyjecy->m_nId);
		
		if (iter != g_WathcTrackID.end()) //说明该轨迹可见
		{
			//存储可见块区域位置
			cv::Rect Rec = cv::Rect(Img.cols + oFFNum / nROwBlobNums *nTrackBlobWidth, \
				(oFFNum%nROwBlobNums)*nTrackBlobHeight, \
				nTrackBlobWidth, nTrackBlobHeight);
			TrackViaslIf trackVisalIfmation;
			trackVisalIfmation.bVisual = true;
			std::string str = "Track:" + to_string(ptyjecy->m_nId);
			if (ptyjecy->m_bActivedByDet)
			{
				str = "T_Nice:" + to_string(ptyjecy->m_nId);
			}
			trackVisalIfmation.str = str;
			trackVisalIfmation.textPoin = cv::Point(TextPoint.x + oFFNum / nROwBlobNums *nTrackBlobWidth, \
				TextPoint.y + (oFFNum%nROwBlobNums) * nTrackBlobHeight);
			trackVisalIfmation.col = cv::Scalar(ptyjecy->m_col[0], ptyjecy->m_col[1], ptyjecy->m_col[2]);
			
			trackVisalIfmation.rec = Rec;
			g_trackMap.insert(make_pair(ptyjecy->m_nId, trackVisalIfmation));
			g_debugIMg(Rec).setTo(cv::Scalar(255, 255, 255));
			g_debugIMg(cv::Rect(Rec.x, Rec.y, Rec.width, 5)).setTo(cv::Scalar(0, 0, 0));
			cv::putText(g_debugIMg, str, trackVisalIfmation.textPoin, 1
				, 1, trackVisalIfmation.col,2);
			oFFNum++;
		}

	}

	//再绘制不可见的部分
	list_iter = m_Usedtracjecy.begin();
	for (; list_iter != m_Usedtracjecy.end(); list_iter++)
	{
		Ctracjecy * ptyjecy = *list_iter;
		iter = find(g_WathcTrackID.begin(), g_WathcTrackID.end(), ptyjecy->m_nId);

		if (iter == g_WathcTrackID.end()) //说明该轨迹不可见
		{
			//存储不可见块区域位置
			cv::Rect Rec = cv::Rect(Img.cols + oFFNum / nROwBlobNums *nTrackBlobWidth,\
				(oFFNum%nROwBlobNums)*nTrackBlobHeight, \
				nTrackBlobWidth, nTrackBlobHeight);
			TrackViaslIf trackVisalIfmation;
			trackVisalIfmation.bVisual = false;
			std::string str;
			if (ptyjecy->m_bActivedByDet)
			{
				str = "T_Nice:" + to_string(ptyjecy->m_nId);
			}
			else
			{
				 str = "Track:" + to_string(ptyjecy->m_nId);
			}
			
			trackVisalIfmation.str = str;
			trackVisalIfmation.textPoin = cv::Point(TextPoint.x + oFFNum / nROwBlobNums *nTrackBlobWidth,\
				TextPoint.y + (oFFNum%nROwBlobNums)*nTrackBlobHeight);
			trackVisalIfmation.col = cv::Scalar(ptyjecy->m_col[0], ptyjecy->m_col[1], ptyjecy->m_col[2]);
			trackVisalIfmation.rec = Rec;

			g_trackMap.insert(make_pair(ptyjecy->m_nId, trackVisalIfmation));

			g_debugIMg(Rec).setTo(cv::Scalar(128, 128, 128));
			g_debugIMg(cv::Rect(Rec.x, Rec.y, Rec.width, 5)).setTo(cv::Scalar(0, 0, 0));
			cv::putText(g_debugIMg, str, trackVisalIfmation.textPoin, 1
				, 1, trackVisalIfmation.col, 2);
			oFFNum++;
		}

	}

	
	//绘制+-轨迹
	g_TrackLenRec = cv::Rect(Img.cols + oFFNum / nROwBlobNums *nTrackBlobWidth,\
		(oFFNum%nROwBlobNums)*nTrackBlobHeight, nTrackBlobWidth, nTrackBlobHeight);
	oFFNum++;
	g_debugIMg(g_TrackLenRec).setTo(cv::Scalar(255, 255, 255));
	g_debugIMg(cv::Rect(g_TrackLenRec.x + g_TrackLenRec.width/3, g_TrackLenRec.y,
		5, g_TrackLenRec.height)).setTo(cv::Scalar(0, 0, 0));
	g_debugIMg(cv::Rect(g_TrackLenRec.x + g_TrackLenRec.width *2/ 3, g_TrackLenRec.y,
		5, g_TrackLenRec.height)).setTo(cv::Scalar(0, 0, 0));

	cv::putText(g_debugIMg, "-", cv::Point(g_TrackLenRec.x, g_TrackLenRec.y + 25), 1.5
		, 1.5, cv::Scalar(0, 0, 255), 2);

	cv::putText(g_debugIMg, "+", cv::Point(g_TrackLenRec.x + g_TrackLenRec.width * 2 / 3 +5
		, g_TrackLenRec.y + 25), 1.5
		, 1.5, cv::Scalar(0, 0, 255), 2);
	g_debugIMg(cv::Rect(g_TrackLenRec.x, g_TrackLenRec.y,
		g_TrackLenRec.width, 5)).setTo(cv::Scalar(0, 0, 0));

	std::string std_MaxLeng = to_string(g_VilsuMaxLenth);
	cv::putText(g_debugIMg, std_MaxLeng, \
		cv::Point(g_TrackLenRec.x + g_TrackLenRec.width * 1 / 3 + 5
		, g_TrackLenRec.y + 25), 1.5
		, 1.5, cv::Scalar(0, 0, 255), 2);

	g_debugIMg4mouseMessage = g_debugIMg.clone();

	//遍历所有轨迹，并进行绘制
	g_fscale = fscale;

	list_iter = m_Usedtracjecy.begin();
	for (; list_iter != m_Usedtracjecy.end(); list_iter++)
	{
		Ctracjecy * ptyjecy = *list_iter;

		iter = find(g_WathcTrackID.begin(), g_WathcTrackID.end(), ptyjecy->m_nId);

		//只显示可见轨迹
		if (iter != g_WathcTrackID.end())
		{
			ptyjecy->DrawTrajecy(&g_debugIMg, fscale, g_VilsuMaxLenth);
		}
		
	}
	//
	g_Usedtracjecy = &m_Usedtracjecy;
	cv::setMouseCallback("轨迹", OnMouse4VisualTrackChoose, 0);
	cv::imshow("轨迹", g_debugIMg);

}
#endif

//核对当前有效轨迹参与了匹配，有对应的匹配结果（匹配上或匹配不上）
bool CTrajecyManage::CheckTrajecyMatchPair(const TrackPair* pTrackpairs)
{
	vector<int> trajecyAddressVecs;
	vector<int>::iterator iter;
	bool bCheckSuccess = true;

	list<Ctracjecy *>::iterator listiter = m_Usedtracjecy.begin();
	for (; listiter != m_Usedtracjecy.end(); listiter++)
	{
		Ctracjecy * ptyjecy = *listiter;

		for (int j = 0; j < min(ptyjecy->m_nTrackLength, MAX_TRAJECY_LENGTH); j++)
		{
			if (ptyjecy->m_Points[j].x < 0 || ptyjecy->m_Points[j].y < 0
				|| ptyjecy->m_Points[j].naddressId != ptyjecy->m_naddressId)
			{
				cout << ptyjecy->m_Points[j].x << "," << ptyjecy->m_Points[j].y << ","
					<< ptyjecy->m_Points[j].naddressId << "," << ptyjecy->m_naddressId << endl;
				CHECK_THROWCRASH(true, 轨迹的数据存在错误！！！);
			}
		}

	}

	//遍历匹配结果：得到哪些轨迹参与了匹配
	for (int i = 0; i < (int)pTrackpairs->CornerMatchRes.size(); i++)
	{
		int naddressId = pTrackpairs->CornerMatchRes[i].inputPoints.naddressId;

		//说明该点已经有归属的轨迹
		if (-1 != naddressId)
		{
			trajecyAddressVecs.push_back(naddressId);
		}
	}


	//遍历所有轨迹，观察有效果的轨迹中是否都参与了匹配:
	//这要求当前帧跟踪的轨迹对应的点要参与到角点匹配跟踪中去！！
	listiter = m_Usedtracjecy.begin();
	for (; listiter != m_Usedtracjecy.begin(); listiter++)
	{
		Ctracjecy *pTrajecy = *listiter;

		iter = find(trajecyAddressVecs.begin(), trajecyAddressVecs.end(), pTrajecy->m_naddressId);

		if (iter == trajecyAddressVecs.end())
		{
			bCheckSuccess = false;

			CHECK_THROWCRASH(true, This trajecy have not joined mathced !!!);

			break;
		}
	}


	return bCheckSuccess;
}

void CTrajecyManage::updataTrajectoy(const TrackPair* pTrackpairs)
{

	//pTrackpairs包含2部分：{1：先去轨迹点的匹配结果；2：新提取点的匹配轨迹结果}
	//遍历前后2帧所有匹配结果的点
	vector<CornerMatch>::const_iterator iter = pTrackpairs->CornerMatchRes.begin();
	int nBit = MAX_TRAJECY_LENGTH - 1; //

	for (; iter != pTrackpairs->CornerMatchRes.end(); iter++)
	{
		const CornerMatch *pCornerMatch = &(*iter);
		
		//观察该匹配是否归属哪个轨迹
		int naddressId = pCornerMatch->inputPoints.naddressId;

		
		//说明该点已经有归属的轨迹
		if (-1 != naddressId)
		{

			if (naddressId < 0 || naddressId >= m_nMaxTrajNum)
			{
#ifdef CHECK_TRACK_RUN
				CHECK_THROWCRASH((naddressId < 0 || naddressId >= m_nMaxTrajNum), naddressId越界);
#endif
				continue;
			}

			//找到该匹配点属于轨迹池中的哪条轨迹
			Ctracjecy* ptracjecy = m_pTrackjecyPool + naddressId;

#ifdef CHECK_TRACK_RUN
			CHECK_THROWCRASH((0 == ptracjecy->m_nTrackLength), 更新的轨迹长度为0);
#endif
			//如果轨迹点匹配成功，则轨迹增长
			if (pCornerMatch->status)
			{
				//等价于ptracjecy->m_nTrackLength %nBit
				int nStoreIndex = ptracjecy->m_nTrackLength & nBit ;

				//添加轨迹点
#ifdef CHECK_TRACK_RUN
				CHECK_THROWCRASH((nStoreIndex < 0 || nStoreIndex >= MAX_TRAJECY_LENGTH), 长度索引越界！！);
#endif
				ptracjecy->m_Points[nStoreIndex] = (pCornerMatch->MatchPoints);

				//设置该点为当前轨迹索引
				ptracjecy->m_Points[nStoreIndex].naddressId = ptracjecy->m_naddressId;

				ptracjecy->m_nTrackLength++;

			}
			else
			{
				ptracjecy->m_nTrakMiss++;
			}
		}
		else//说明匹配点是新增的
		{
			//添加新轨迹,确保是匹配成功的点
			if (pCornerMatch->status)
			{
				newTrajectoy(pCornerMatch->inputPoints, pCornerMatch->MatchPoints);
			}
			
		}

	}

}


void CTrajecyManage::Freetracjecy(Ctracjecy* ptracjecy)
{
	//清空轨迹数据
	ptracjecy->clearTrajecy();

	//将该轨迹进入空闲状态，并将其的地址空间放入到m_Freetracjecy
	//后期新增轨迹从m_Freetracjecy中取
	m_Freetracjecy.push_back(ptracjecy);

}

//申请空闲的轨迹空间
Ctracjecy* CTrajecyManage::applyFreetracjecy()
{
	//是否存在空闲的轨迹
	if (m_Freetracjecy.empty())
	{
		return  NULL;
	}
	else
	{
		//使用最后一个
		Ctracjecy* ptracjecy = m_Freetracjecy.back();
		m_Freetracjecy.pop_back();

		//放入到使用的轨迹当中去
		m_Usedtracjecy.push_back(ptracjecy);

		return  ptracjecy;
	}
}

//产生新的轨迹空间
void CTrajecyManage::newTrajectoy(const Pointf &inputPoints, const Pointf &MatchPoints)
{
	//申请闲置的轨迹空间
	Ctracjecy *pTrajecy = applyFreetracjecy();

	//如果申请成功
	if (pTrajecy)
	{

#ifdef CHECK_TRACK_RUN
		CHECK_THROWCRASH(-1 != pTrajecy->m_nId, New Trackjecy ID != -1);
		CHECK_THROWCRASH(pTrajecy->m_nTrackLength, New Trackjecy 长度 不 为 0);
#endif
		pTrajecy->m_Points[0] = inputPoints;
		pTrajecy->m_Points[0].naddressId = pTrajecy->m_naddressId;

		pTrajecy->m_Points[1] = MatchPoints;
		pTrajecy->m_Points[1].naddressId = pTrajecy->m_naddressId;

		pTrajecy->m_nTrackLength = 2;
		pTrajecy->m_nId = m_uCurId;
		pTrajecy->m_bValid = true;

		m_uCurId++;

	}

}

//删除轨迹
void CTrajecyManage::DelTrajectoys( bool bDelAll)
{
	//遍历所有轨迹（含使用和空闲的（轨迹空闲的：仍保留轨迹地址空间））
	//del:只针对当前正在使用的，但被判别当前轨迹跟踪失败的
	list<Ctracjecy *>::iterator iter = m_Usedtracjecy.begin();

	for (; iter != m_Usedtracjecy.end();)
	{
		Ctracjecy *pTrajecy = *iter;

#ifdef CHECK_TRACK_RUN
		CHECK_THROWCRASH(-1 == pTrajecy->m_nId, 使用的轨迹其ID=-1);
#endif

		if (bDelAll)
		{
			//释放该轨迹（即该轨迹数据清空，轨迹地址空间可被后期新的轨迹所使用）
			Freetracjecy(pTrajecy);
			iter = m_Usedtracjecy.erase(iter);
		}
		else
		{
			bool bValid = pTrajecy->CheckTrajecyIsValid();

			//如果轨迹跟踪失败了
			if (!bValid)
			{
				//释放该轨迹（即该轨迹数据清空，轨迹地址空间可被后期新的轨迹所使用）
				Freetracjecy(pTrajecy);
				iter = m_Usedtracjecy.erase(iter);
			}
			else
			{
				iter++;
			}
		}

	}

}
