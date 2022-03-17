#ifndef SSDECTOR_H
#define SSDECTOR_H

#include "basedetector.h"
#include <QMap>

class SSDector : public BaseDetector
{
public:
    SSDector();
    ~SSDector();

    int initModel(const std::string modelNet, const std::string modelWeight="");

    int initDetectorParameters(const int dataWidth, const int dataHeight,
                               const float confidenceThreshold, const QMap<int, QString> &labels);

    void processDetect(const cv::Mat &inputImage, std::vector<Detect2dBox> &objectRect);

private:

    cv::Mat detect(const cv::Mat &image);

    void processDetectionObject(const cv::Mat& roi, const cv::Mat& detectionObjects,
                                const int topX, const int topY,
                                std::vector<Detect2dBox> &objectRect);

    void showDetection(cv::Mat &image, std::vector<Detect2dBox> &objectRect);

    void initData();

private:

    //Initialize network
    cv::dnn::Net net;

    int inputDataWidth;
    int inputDataHeight;
    float confidenceThreshold;

private:

     void saveConfig();
     void loadConfig();

};

#endif // SSDECTOR_H
