#ifndef CAFFEDETECTOR_H
#define CAFFEDETECTOR_H
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <map>

#include "cnn_data_type.h"

class BaseDetector
{
public:
    BaseDetector();
    virtual ~BaseDetector();

    virtual int initModel(const std::string modelNet, const std::string modelWeight="") = 0;
    virtual int initDetectorParameters(const int dataWidth, const int dataHeight,
                                       const float confidenceThreshold, const std::map<int, std::string> &labels) = 0;
    virtual void processDetect(const cv::Mat &inputImage, std::vector<Detect2dBox> &objectRect) = 0;

protected:

    std::map<int, std::string> labels;

    void showDetection(cv::Mat &image, std::vector<Detect2dBox> &objectRect);

private:
    virtual void saveConfig() = 0;
    virtual void loadConfig() = 0;
};

#endif // CAFFEDETECTOR_H
