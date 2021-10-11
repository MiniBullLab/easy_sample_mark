
#include "CornerTrack.h"
#include "opencv2/opencv.hpp"

#ifdef SHOW_IMG
cv::Rect g_RioRec;
cv::Mat g_PreImg;
cv::Mat g_CurImg;
cv::Mat g_Flow;
int g_showFirst = 1;
void GrayTocol(cv::Mat Gray, cv::Mat &Col)
{
	vector<cv::Mat>channels;
	channels.push_back(Gray);
	channels.push_back(Gray);
	channels.push_back(Gray);
	cv::merge(channels, Col);
}

void OnMouseDenseMatch(
	int event,
	int x,
	int y,
	int flags,
	void* param
	)
{
	cv::Mat pre_img, cur_img;
	cv::Mat show_denseImg;
	cv::Mat flowMat = g_Flow.clone();
	GrayTocol(g_PreImg(g_RioRec), pre_img);
	GrayTocol(g_CurImg(g_RioRec), cur_img);


	int nW = pre_img.cols;
	int nH = pre_img.rows;
	float fscale = 5;
	show_denseImg = cv::Mat(nH * 2, nW, CV_8UC3);

	pre_img.copyTo(show_denseImg(cv::Rect(0, 0, nW, nH)));
	cur_img.copyTo(show_denseImg(cv::Rect(0, nH, nW, nH)));

	cv::resize(show_denseImg, show_denseImg, \
		cv::Size(nW *fscale, nH * 2 * fscale), CV_INTER_AREA);


	if (g_showFirst)
	{
		imshow("denseMatch", show_denseImg);
		g_showFirst = 0;
	}


	if (flowMat.data)
	{
		switch (event)
		{
		case CV_EVENT_MOUSEMOVE:
		{
								   break;
		}
		case CV_EVENT_LBUTTONDOWN:
		{

									 cv::Scalar Col(0, 0, 255);

									 circle(show_denseImg, \
										 cv::Point(x, y), \
										 2, Col);

									 cv::Point2f fdxy = flowMat.at<cv::Point2f>(y / fscale, x / fscale);

									 cv::Point2f MatchPoint;
									 string flowstr = "x:" + to_string(fdxy.x);
									 cv::putText(show_denseImg, flowstr, \
										 cv::Point(15, 15), 0.4, 0.4, cv::Scalar(0, 255, 0));
									 flowstr = "y:" + to_string(fdxy.y);
									 cv::putText(show_denseImg, flowstr, \
										 cv::Point(25, 15), 0.4, 0.4, cv::Scalar(0, 255, 0));

									 MatchPoint.x = round(x + fdxy.x);
									 MatchPoint.y = round(y + fdxy.y) + nH*fscale;
									 circle(show_denseImg, MatchPoint, 2, Col, 1);
									 imshow("denseMatch", show_denseImg);


									 break;
		}

		case CV_EVENT_RBUTTONDOWN:
		{
									 imshow("denseMatch", show_denseImg);
									 break;

		}

		default:

			break;

		}
	}
}

#endif

CornerTrack::CornerTrack() :
m_pCornerTracContext(NULL),
m_pFlowMat(NULL)
{
	m_pFlowMat = new cv::Mat[MAX_DENSE_RIO_NUM];
}

CornerTrack::~CornerTrack()
{
    if(m_pFlowMat)
        delete [] m_pFlowMat;
    m_pFlowMat = NULL;
}

/*********************
函数名：CornerTrackProcess
输入参数：pCornerTracRuntime运行上下文
输出：true:跟踪成功
功能：实现前后2帧的角点跟踪

***********************/
bool CornerTrack::CornerTrackProcess(const CornertrackRuntimeContext* pCornerTracRuntime)//角点跟踪
{
	
	m_pCornerTracContext = pCornerTracRuntime;


	//根据上一帧的角点，得出当前帧对应的匹配结果，结果存储在m_TrackMatchPoints
	switch (pCornerTracRuntime->tracktype)
	{
		case DENSE_LK://稠密光流
		{
			//DenseOpticaltrack(pCornerTracRuntime, &m_TrackMatchPoints);
			break;
		}

		case SPARSE_LK://稀疏光流
		{
			SparseOpticaltrack(pCornerTracRuntime, &m_TrackMatchPoints);
			break;
		}

	default:
		break;
	}

	//对匹配结果进行过滤：如采用NCC，RANSAC等方法
	FilterTrack(pCornerTracRuntime, &m_TrackMatchPoints);


	//将landMark点剥离开,因为landmark点不参与轨迹！，仅做前后2帧跟踪
	SplitLpsFromMatcs(&m_TrackMatchPoints, &m_LandkMatchPoints);


	//m_TrackMatchPoints 此时存储了{轨迹点+额外点+角点}的一一对应的匹配结果
#ifdef SHOW_IMG
	ShowMatchResult(pCornerTracRuntime, &m_TrackMatchPoints);
#endif


	return true;
}


#ifdef SHOW_IMG
//匹配匹配结果
void CornerTrack::ShowMatchResult(const CornertrackRuntimeContext* pCornerTracRuntime, \
	TrackPair* pMatchPoints)
{

#ifndef CORNER_SHOW
	return ;
#endif
	//稠密不做显示
	if (!pMatchPoints)
	{
		return;
	}
	if (pMatchPoints->CornerMatchRes.size() < 5)
	{
		return;
	}

	//得到外界举行
	int nMinx = 99999;
	int nMiny = 99999;
	int nMaxX = 0;
	int nMaxY = 0;

	if (!pCornerTracRuntime || !pCornerTracRuntime->preGrayImg || !pCornerTracRuntime->pCurGrayImg
		|| !pCornerTracRuntime->pTrackRios)
	{
		return;
	}

	for (int i = 0; i < pCornerTracRuntime->pTrackRios->size(); i++)
	{
		const cv::Rect2f* pRec = &pCornerTracRuntime->pTrackRios->at(i);

		nMinx = MIN(nMinx, pRec->x);
		nMiny = MIN(nMiny, pRec->y);
		nMaxX = MIN(nMaxX, pRec->x + pRec->width);
		nMaxY = MIN(nMaxY, pRec->y + pRec->height);
	}

	for (int i = 0; i < pMatchPoints->CornerMatchRes.size(); i++)
	{
		const Pointf &inputpt = pMatchPoints->CornerMatchRes[i].inputPoints;

		if (!pMatchPoints->CornerMatchRes[i].status)
		{
			continue;
		}
		nMinx = MIN(inputpt.x, nMinx);
		nMiny = MIN(inputpt.y, nMiny);
		nMaxX = MAX(inputpt.x, nMaxX);
		nMaxY = MAX(inputpt.y, nMaxY);
	}

	if (nMaxX < nMinx)
	{
		return;
	}
	cv::Rect rect;
	int ndx = MAX(120, (nMaxX - nMinx));
	int ndy = MAX(150, (nMaxY - nMiny));
	rect.x = MAX(0, nMinx - ndx);
	rect.y = MAX(0, nMaxY - ndy);
	rect.width = MIN(pCornerTracRuntime->preGrayImg->cols - 1, nMaxX + ndx) - rect.x;
	rect.height = MIN(pCornerTracRuntime->preGrayImg->rows - 1, nMaxY + ndy) - rect.y;
	rect = rect &cv::Rect(0, 0, pCornerTracRuntime->preGrayImg->cols-1, pCornerTracRuntime->preGrayImg->rows-1);
	cv::Point offPoint(rect.x, rect.y);
	cv::Mat pre_img, cur_img;
	cv::Mat show_Img;

	cv::Mat A = (*pCornerTracRuntime->preGrayImg)(rect).clone();
	cv::Mat B = (*pCornerTracRuntime->pCurGrayImg)(rect).clone();
	GrayTocol(A, pre_img);
	GrayTocol(B, cur_img);

	int nW = rect.width;
	int nH = rect.height;

	//上下叠加
	int nTrackBlobHeight = 40;
	int nTrackBlobWidth = 60;
	int nRowBlobNum = MAX(1,nH * 2 / nTrackBlobHeight);
	int nColBlobNum = (cvCeil)((float)pMatchPoints->CornerMatchRes.size() / nRowBlobNum);

	show_Img = cv::Mat(pre_img.rows + cur_img.rows + 1,\
		pre_img.cols + nColBlobNum * nTrackBlobWidth + nColBlobNum, \
		CV_8UC3, cv::Scalar(0, 0, 0));
	

	pre_img.copyTo(show_Img(cv::Rect(0, 0, pre_img.cols, pre_img.rows)));
	cur_img.copyTo(show_Img(cv::Rect(0, pre_img.rows, cur_img.cols, cur_img.rows)));


	if (pCornerTracRuntime->pTrackRios)
	{
		for (int i = 0; i < pCornerTracRuntime->pTrackRios->size(); i++)
		{
			const cv::Rect2f* pRec = &pCornerTracRuntime->pTrackRios->at(i);
			cv::Point A = cv::Point(pRec->x - offPoint.x, pRec->y - offPoint.y);
			A.x = MAX(0, MIN(show_Img.cols - 1, A.x));
			A.y = MAX(0, MIN(show_Img.rows - 1, A.y));
			cv::Point B = cv::Point(pRec->x + pRec->width - offPoint.x, pRec->y + pRec->height - offPoint.y);
			B.x = MAX(0, MIN(show_Img.cols - 1,B.x));
			B.y = MAX(0, MIN(show_Img.rows - 1, B.y));
			cv::rectangle(show_Img,A,B, \
				cv::Scalar(0, 0, 255));
		}
	}

	int nMatchNum = 0;
	for (int i = 0; i < pMatchPoints->CornerMatchRes.size(); i++)
	{
		const Pointf &inputpt = pMatchPoints->CornerMatchRes[i].inputPoints;

		bool binScedRec = false;
		if (pCornerTracRuntime->pTrackRios->size()>1 &&
			inputpt.x >= pCornerTracRuntime->pTrackRios->at(1).x
			&& inputpt.y >= pCornerTracRuntime->pTrackRios->at(1).y
			&& inputpt.x < pCornerTracRuntime->pTrackRios->at(1).x + pCornerTracRuntime->pTrackRios->at(1).width
			&& inputpt.y < pCornerTracRuntime->pTrackRios->at(1).y + pCornerTracRuntime->pTrackRios->at(1).height)
		{
			binScedRec = true;
		}

		if (pMatchPoints->CornerMatchRes[i].status)
		{
			const Pointf &Matchpt = pMatchPoints->CornerMatchRes[i].MatchPoints;
			cv::Scalar col = cv::Scalar(rand() % 255, rand() % 255, rand() % 255);
			cv::Point A = cv::Point(inputpt.x - offPoint.x, inputpt.y - offPoint.y);
			A.x = MAX(0, MIN(show_Img.cols - 1, A.x));
			A.y = MAX(0, MIN(show_Img.rows - 1, A.y));
			cv::Point B = cv::Point(Matchpt.x - offPoint.x, Matchpt.y + nH - offPoint.y);
			B.x = MAX(0, MIN(show_Img.cols - 1, B.x));
			B.y = MAX(0, MIN(show_Img.rows - 1, B.y));
			cv::line(show_Img, A, B, col);
			std::string str = "(" + to_string(int(Matchpt.x - inputpt.x)) + \
				"," + to_string(int(Matchpt.y - inputpt.y)) + ")";

			if (binScedRec)
			{
				cv::Rect rec = cv::Rect(nW + nMatchNum / nRowBlobNum *nTrackBlobWidth, \
					nMatchNum % nRowBlobNum * nTrackBlobHeight, nTrackBlobWidth, nTrackBlobHeight);
				rec &= cv::Rect(0, 0, show_Img.cols, show_Img.rows);
				show_Img(rec).setTo(255);
			}

			 A = cv::Point(nW + nMatchNum / nRowBlobNum *nTrackBlobWidth, \
				nMatchNum % nRowBlobNum * nTrackBlobHeight + nTrackBlobHeight / 3);
			A.x = MAX(0, MIN(show_Img.cols - 1, A.x));
			A.y = MAX(0, MIN(show_Img.rows - 1, A.y));

			cv::putText(show_Img, str, A, 1, 1, col);
			nMatchNum++;
		}
		else
		{
			/*cv::circle(show_Img, cv::Point(inputpt.x - offPoint.x, inputpt.y - offPoint.y), \
				2, cv::Scalar(rand() % 255, rand() % 255, rand() % 255));*/
		}

	}

	cv::imshow("CorMatch", show_Img);

	//
	if (!m_LandkMatchPoints.CornerMatchRes.empty())
	{
		cv::Mat ful_pre_img, ful_cur_img;
		GrayTocol((*pCornerTracRuntime->preGrayImg), ful_pre_img);
		GrayTocol((*pCornerTracRuntime->pCurGrayImg), ful_cur_img);

		cv::Mat  combineImg(ful_pre_img.rows * 2, ful_pre_img.cols, CV_8UC3);

		for (int i = 0; i < m_LandkMatchPoints.CornerMatchRes.size(); i++)
		{
			Pointf *p1 = &m_LandkMatchPoints.CornerMatchRes.at(i).inputPoints;
			Pointf *p2 = &m_LandkMatchPoints.CornerMatchRes.at(i).MatchPoints;
			bool bStatues = m_LandkMatchPoints.CornerMatchRes.at(i).status;
			cv::Scalar col = bStatues ? CV_RGB(0, 255, 0) : CV_RGB(0, 0, 0);

			cv::circle(ful_pre_img, cv::Point(p1->x, p1->y), 2, col);

			if (bStatues)
			{
				cv::circle(ful_cur_img, cv::Point(p2->x, p2->y), 2, CV_RGB(255, 0, 0));
			}
		}
		ful_pre_img.copyTo(combineImg(cv::Rect(0, 0, ful_pre_img.cols, ful_pre_img.rows)));
		ful_cur_img.copyTo(combineImg(cv::Rect(0, ful_cur_img.rows, ful_cur_img.cols, ful_cur_img.rows)));


		cv::imshow("combineImg", combineImg);
		cv::waitKey(10);
	}

}
#endif

//匹配角点Filter
void CornerTrack::FilterTrack(const CornertrackRuntimeContext* pCornerTracRuntime, \
							  TrackPair* pMatchPoints)
{
	//如果没有过滤规则，就直接返回
	if (pCornerTracRuntime->FilterType.empty())
	{
		return;
	}

	for (int i = 0; i < (int)pCornerTracRuntime->FilterType.size(); i++)
	{
		int FilterType = pCornerTracRuntime->FilterType[i];

		switch (FilterType)
		{
		case Dir_FILTER://方向过滤
		{
							
			break;
		}

		case RANSAC_FILTER://
		{
			FilterRanSac(pMatchPoints);
			break;
		}

		default:
			break;
		}

	}
}


void CornerTrack::SplitLpsFromMatcs(TrackPair* pMatchPoints, TrackPair* pLandMarks)
{
	pLandMarks->CornerMatchRes.clear();
	vector<CornerMatch>::iterator MacthIter = pMatchPoints->CornerMatchRes.begin();
	for (; MacthIter != pMatchPoints->CornerMatchRes.end();)
	{
		if (MacthIter->inputPoints.bLandMark)
		{
			pLandMarks->CornerMatchRes.push_back(*MacthIter);
			MacthIter = pMatchPoints->CornerMatchRes.erase(MacthIter);
		}
		else
		{
			MacthIter++;
		}
	}

}

//fliter2_Ransac：
void CornerTrack::FilterRanSac(TrackPair* pMatchPoints)
{
	vector<uchar> Ransacstatus;
	vector<cv::Point2f> p0, p1;
	const int nCNum = (int)pMatchPoints->CornerMatchRes.size();
	const int nRansacNumThresh = 8;
	p0.reserve(nCNum);
	p1.reserve(nCNum);

	map<int, int> TosrcIndexMap;
	for (int i = 0; i < nCNum; i++)
	{
		CornerMatch * pCornerMatchRes = &(pMatchPoints->CornerMatchRes[i]);

		if (pCornerMatchRes->status)
		{
			p0.push_back(cv::Point2f(pCornerMatchRes->inputPoints.x, pCornerMatchRes->inputPoints.y));
			p1.push_back(cv::Point2f(pCornerMatchRes->MatchPoints.x, pCornerMatchRes->MatchPoints.y));
			TosrcIndexMap.insert(make_pair((int)TosrcIndexMap.size(), (int)i));
		}

	}

	if (p0.size() > nRansacNumThresh)
	{
		//是否为内点
		vector<uchar> inlierStatus;

		//返回的是基础矩阵
		//cv::Mat Fundmental = cv::findFundamentalMat(p0, p1, RansacStatus, CV_FM_RANSAC);

		//返回变换矩阵
		cv::Mat homography;
        homography = cv::findHomography(p0, p1, cv::RANSAC, 3, inlierStatus);

		//vector<cv::point2f>src,dst 求得变化后的矩阵
		//cv::perspectiveTransform(src, dst, homography)

		//如果被Ransac过滤掉！
		for (int i = 0; i < (int)inlierStatus.size(); i++)
		{
			if (!inlierStatus[i])
			{
				//设置原始对应匹配为false
				pMatchPoints->CornerMatchRes[TosrcIndexMap[i]].status = false;
			}
		}
	}



}

/*********************
函数名：DenseOpticaltrack
输入参数：CornertrackRuntime运行上下文
输出：无
功能：实现前后2帧的跟踪

***********************/
bool CornerTrack::DenseOpticaltrack(const CornertrackRuntimeContext* pCornerTracRuntime, \
	TrackPair *pTrackMatchPoint)//稠密光流
{

	cv::Point offpoint(0, 0);
	const vector<cv::Rect2f>* pTracRioRec = pCornerTracRuntime->pTrackRios;
	int nDensepixelNum = 0;
	int nIndex = 0;

	//对Rio区域进行稠密光流计算
	if (!pTracRioRec || pTracRioRec->empty())
	{
		pTrackMatchPoint->CornerMatchRes.clear();
		return false;
	}

	assert(pTracRioRec->size() <= MAX_DENSE_RIO_NUM);

	//遍历所有RIo,计算得到一个需要的稠密点
	for (int i = 0; i < (int)pTracRioRec->size(); i++)
	{
		const cv::Rect2f *pRec = &pTracRioRec->at(i);
		nDensepixelNum += int(pRec->width  * pRec->height);
	}

	pTrackMatchPoint->CornerMatchRes.resize(nDensepixelNum);


	//遍历所有RIo
	for (int m = 0; m < (int)pTracRioRec->size(); m++)
	{
		const cv::Rect2f* pRec = &pTracRioRec->at(m);

		offpoint = cv::Point((int)pRec->x, (int)pRec->y);

		cv::calcOpticalFlowFarneback(
			(*pCornerTracRuntime->preGrayImg)(cv::Rect((int)pRec->x, (int)pRec->y, (int)pRec->width, (int)pRec->height)), \
			(*pCornerTracRuntime->pCurGrayImg)(cv::Rect((int)pRec->x, (int)pRec->y, (int)pRec->width, (int)pRec->height)), \
			m_pFlowMat[m], 0.5, 4, 7, 3, 5, 1.2, 0);



		//获得稠密光流的匹配结果[flow =dst -src 的偏移量]
		for (int j = 0; j < m_pFlowMat[m].rows; j++)
		{
			for (int i = 0; i < m_pFlowMat[m].cols; i++)
			{
				cv::Point2f fdxy = m_pFlowMat[m].at<cv::Point2f>(j, i);

				Pointf srcPoint;
				srcPoint.x = float(i + offpoint.x);
				srcPoint.y = float(j + offpoint.y);

				Pointf srcMatchPoint;
				srcMatchPoint.x = i + offpoint.x + fdxy.x;
				srcMatchPoint.y = j + offpoint.y + fdxy.y;

				pTrackMatchPoint->CornerMatchRes[nIndex].inputPoints = srcPoint;
				pTrackMatchPoint->CornerMatchRes[nIndex].MatchPoints = srcMatchPoint;
				pTrackMatchPoint->CornerMatchRes[nIndex].status = true;
				nIndex++;

			}
		}

	}


#ifdef SHOW_IMG

	//仅仅显示第一个RIO区域的稠密光流
	int nw = pCornerTracRuntime->preGrayImg->cols;
	int nh = pCornerTracRuntime->preGrayImg->rows;

	g_RioRec = cv::Rect(pTracRioRec->at(0).x, pTracRioRec->at(0).y, pTracRioRec->at(0).width, pTracRioRec->at(0).height);
	g_CurImg = pCornerTracRuntime->pCurGrayImg->clone();
	g_PreImg = pCornerTracRuntime->preGrayImg->clone();
	g_Flow = m_pFlowMat->clone();
	g_showFirst = 1;

	cv::namedWindow("denseMatch");
	cv::setMouseCallback("denseMatch", OnMouseDenseMatch, 0);

#endif

	return true;
}


bool CornerTrack::SparseOpticaltrack(const CornertrackRuntimeContext* pCornerTracRuntime, \
	TrackPair *pTrackMatchPoints)//稀疏光流
{
	if (pCornerTracRuntime->pcorners->empty())
	{
		pTrackMatchPoints->CornerMatchRes.clear();
		return false;
	}

	cv::Size winSize = cv::Size(15, 15);
	int maxLevel = 3;


	//如果只有一个目标，在构建LK时，只需要感兴趣区域即可
	bool bOnlyOneRoiLKStatus = (1 == pCornerTracRuntime->pTrackRios->size());
	cv::Rect LKRio = cv::Rect(0, 0, pCornerTracRuntime->preGrayImg->cols - 1,
		pCornerTracRuntime->preGrayImg->rows - 1);

	//目标是否仅有一个，启动了LKRIO操作！
	if (bOnlyOneRoiLKStatus)
	{
		const float fpadding = 0.35f;
		const cv::Rect2f &rect = pCornerTracRuntime->pTrackRios->at(0);
		float fexpandx = fpadding * rect.width;
		float fexpandy = fpadding * rect.height;

		LKRio.x = MAX(0, int(rect.x - fexpandx));
		LKRio.y = MAX(0, int(rect.y - fexpandy));

		LKRio.width = MIN(pCornerTracRuntime->preGrayImg->cols - 1, \
			int(rect.x + rect.width + fexpandx)) - LKRio.x;
		LKRio.height = MIN(pCornerTracRuntime->preGrayImg->rows - 1, \
			int(rect.y + rect.height + fexpandy)) - LKRio.y;

		LKRio = LKRio & cv::Rect(0, 0, \
			pCornerTracRuntime->preGrayImg->cols, pCornerTracRuntime->preGrayImg->rows);
	}

	if (MAX(LKRio.width, LKRio.height) < winSize.width + 1)
	{
		pTrackMatchPoints->CornerMatchRes.clear();
		return false;
	}

	cv::TermCriteria termcrit(\
		cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 15, 0.3);

	//在人脸小场景中，由远靠近时，人脸变大，会造成耗时增加
	const float fNormalSize = 80.0;
	float fsacle = 1;
	cv::Mat PreGray, gray;


	if (bOnlyOneRoiLKStatus && MAX(LKRio.width, LKRio.height) > fNormalSize)
	{
		fsacle = MAX(LKRio.width, LKRio.height) / fNormalSize;
		cv::Size imgSiz = cv::Size(int(LKRio.width / fsacle), int(LKRio.height / fsacle));
		cv::resize((*pCornerTracRuntime->preGrayImg)(LKRio), PreGray, imgSiz, cv::INTER_NEAREST);
		cv::resize((*pCornerTracRuntime->pCurGrayImg)(LKRio), gray, imgSiz, cv::INTER_NEAREST);
	}
	else
	{

		PreGray = (*pCornerTracRuntime->preGrayImg)(LKRio);
		gray = (*pCornerTracRuntime->pCurGrayImg)(LKRio);
	}

	//上一阵的角点
	vector<cv::Point2f> vPtfsLast;
	vector<cv::Point2f> vPtfsNow;
	vector<uchar> vStatus_uc;
	vector<float> vErr_f;

	//根据上一帧得到的角点个数，设置输出的匹配对数
	const int CornerNum = (int)pCornerTracRuntime->pcorners->size();
	pTrackMatchPoints->CornerMatchRes.resize(pCornerTracRuntime->pcorners->size());
	vPtfsLast.reserve(pCornerTracRuntime->pcorners->size());

	//数据类型转换;得到上一帧的角点
	for (int i = 0; i < CornerNum; i++)
	{
		//获得LK需要的上一帧角点个数
		const  Pointf  &pt = pCornerTracRuntime->pcorners->at(i);

		cv::Point2f ptInmg;

		ptInmg.x = (pt.x - LKRio.x) / fsacle;
		ptInmg.x = MIN(MAX(0, ptInmg.x), gray.cols - 1);
		ptInmg.y = (pt.y - LKRio.y) / fsacle;
		ptInmg.y = MIN(MAX(0, ptInmg.y), gray.rows - 1);


		vPtfsLast.push_back(ptInmg);

		//naddressId值必须赋！！
		pTrackMatchPoints->CornerMatchRes[i].inputPoints = Pointf(ptInmg.x, ptInmg.y, \
			pt.naddressId, pt.bLandMark);
	}


	//前向传播，计算A->B的LK
	cv::calcOpticalFlowPyrLK(PreGray, gray, vPtfsLast, vPtfsNow,
		vStatus_uc, vErr_f, winSize, maxLevel, termcrit, 0, 0.001);


#ifdef CHECK_TRACK_RUN
	CHECK_THROWCRASH((vPtfsNow.size() != vPtfsLast.size()), " vPtfsNow与vPtfsNow不匹配！");
#endif 


	//获得输出时的匹配对的匹配数据
	for (int i = 0; i < (int)vPtfsNow.size(); i++)
	{
		Pointf  MatchPoint(vPtfsNow[i].x, vPtfsNow[i].y);
		pTrackMatchPoints->CornerMatchRes[i].MatchPoints = MatchPoint;

	}

	map<int, int> nowgoodtosrcIndex;
	vector<cv::Point2f> vPtfsNow_match;
	vPtfsNow_match.reserve(vStatus_uc.size());

#ifdef CHECK_TRACK_RUN
	CHECK_THROWCRASH((vStatus_uc.size() != vPtfsLast.size()), " states 与输入不匹配！");
#endif

	//将匹配成功的点进行保留，同时记录：匹配成功点的索引 映射 原始输入点的索引
	for (int i = 0; i < (int)vStatus_uc.size(); i++)
	{
		if (vStatus_uc[i])
		{
			//如果匹配点溢出图像也认为匹配失败
			int nx = (int)pTrackMatchPoints->CornerMatchRes[i].MatchPoints.x;
			int ny = (int)pTrackMatchPoints->CornerMatchRes[i].MatchPoints.y;
			if (nx < 0 || ny < 0 || nx > gray.cols - 1 || ny > gray.rows - 1)
			{
				vStatus_uc[i] = false;
				continue;
			}

			vPtfsNow_match.push_back(vPtfsNow[i]);
			nowgoodtosrcIndex.insert(make_pair((int)nowgoodtosrcIndex.size(), i));
		}
	}


	//back方向传播
	vector<uchar> vStatus_uc_back;
	vector<float> vErr_f_back;
	vector<cv::Point2f> vPtfsNow_back;

	if (vPtfsNow_match.empty())
	{
		for (int i = 0; i < (int)vStatus_uc.size(); i++)
		{
			pTrackMatchPoints->CornerMatchRes[i].status = false;
		}

		return false;
	}

	//计算B->A的LK，逆向光流
	cv::calcOpticalFlowPyrLK(gray, PreGray, vPtfsNow_match, vPtfsNow_back,
		vStatus_uc_back, vErr_f_back, winSize, maxLevel, termcrit, 0, 0.001);

	const float fbDisThresh = 2;

	//观察逆向匹配清空：正确的匹配点，在经过一次正，反LK该等于其自身
	for (int i = 0; i < (int)vStatus_uc_back.size(); i++)
	{
		int MapSrcIndex = nowgoodtosrcIndex[i];

		//观察光流成功的点
		if (vStatus_uc_back[i])
		{
			float fbDis = fabs(vPtfsNow_back[i].x - vPtfsLast[MapSrcIndex].x)
				+ fabs(vPtfsNow_back[i].y - vPtfsLast[MapSrcIndex].y);

			//理想情况:A->B->A', A==A'
			if (fbDis > fbDisThresh)
			{
				vStatus_uc[MapSrcIndex] = false;
			}
		}
		else
		{
			vStatus_uc[MapSrcIndex] = false;
		}
	}


	//匹配结果
	if (bOnlyOneRoiLKStatus)
	{
		for (int i = 0; i < (int)vStatus_uc.size(); i++)
		{
			pTrackMatchPoints->CornerMatchRes[i].inputPoints.x *= fsacle;
			pTrackMatchPoints->CornerMatchRes[i].inputPoints.x += LKRio.x;

			pTrackMatchPoints->CornerMatchRes[i].inputPoints.y *= fsacle;
			pTrackMatchPoints->CornerMatchRes[i].inputPoints.y += LKRio.y;

			pTrackMatchPoints->CornerMatchRes[i].MatchPoints.x *= fsacle;
			pTrackMatchPoints->CornerMatchRes[i].MatchPoints.x += LKRio.x;

			pTrackMatchPoints->CornerMatchRes[i].MatchPoints.y *= fsacle;
			pTrackMatchPoints->CornerMatchRes[i].MatchPoints.y += LKRio.y;

			pTrackMatchPoints->CornerMatchRes[i].status = (0 == vStatus_uc[i]) ? false : true;

		}

	}
	else
	{
		for (int i = 0; i < (int)vStatus_uc.size(); i++)
		{
			pTrackMatchPoints->CornerMatchRes[i].status = (0 == vStatus_uc[i]) ? false : true;
		}
	}

#ifdef PRINTF_INF
	int nSuccessNum = 0;
	for (int i = 0; i < vStatus_uc.size(); i++)
	{
		if (vStatus_uc[i])
		{
			nSuccessNum++;
		}
	}
	cout << "LK数目:" << vStatus_uc.size() << ",其中成功数目" << nSuccessNum << endl;
#endif

	return true;

}


/*********************
函数名：CalMajorFlowvector
输入参数：pFlowMat：跟踪点的匹配运动矢量
输出：运动矢量集合的中值
功能：统计所有运动矢量的中值

***********************/
flowVec CornerTrack::CalMajorFlowvector(cv::Mat *pFlowMat)
{
	flowVec MajiorflowVec(0, 0);
	assert(pFlowMat);

	vector<float> fx;
	vector<float> fy;

	int nNum = pFlowMat->rows * pFlowMat->cols;
	fx.resize(nNum);
	fy.resize(nNum);

	int nIndex = 0;

	//分别统计所有的下x,y方向上的运动矢量
	for (int j = 0; j < pFlowMat->rows; j++)
	{
		for (int i = 0; i < pFlowMat->cols; i++)
		{
			cv::Point2f fdxy = pFlowMat->at<cv::Point2f>(j, i);

			//x方向
			fx[nIndex] = fdxy.x;

			//Y方向
			fy[nIndex++] = fdxy.y;
		}
	}

	//排序求中值
	sort(fx.begin(), fx.end());
	sort(fy.begin(), fy.end());

	//将中值得到的矢量作为主矢量
	MajiorflowVec.x = fx[fx.size() / 2];
	MajiorflowVec.y = fy[fy.size() / 2];

	return MajiorflowVec;
}
