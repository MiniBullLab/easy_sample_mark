#ifndef YOLOV5DECTOR_H
#define YOLOV5DECTOR_H

#include "basedetector.h"
#include <QMap>

class YoloV5Dector : public BaseDetector
{
public:
    YoloV5Dector();
    ~YoloV5Dector();

    int initModel(const std::string modelNet, const std::string modelWeight="");

    int initDetectorParameters(const int dataWidth, const int dataHeight,
                               const float confidenceThreshold, const QMap<int, QString> &labels);

    void processDetect(const cv::Mat &inputImage, std::vector<Detect2dBox> &objectRect);

private:

    //Initialize network
    cv::dnn::Net net;

    int inputDataWidth;
    int inputDataHeight;
    float confidenceThreshold;

    std::vector<std::vector<float> > anchors;
    std::vector<float> stride;
    float nmsThreshold;
    float objThreshold;

private:

    void initData();

    int postprocess(const cv::Size &srcSize, const std::vector<cv::Mat> &outputs, std::vector<Detect2dBox> &det_results);
    std::vector<std::vector<float>> applyNMS(std::vector<std::vector<float>>& boxes,
                                    const float thres);

     void saveConfig();
     void loadConfig();
};

#endif // YOLOV5DECTOR_H
