#ifndef CAFFEDETECTOR_H
#define CAFFEDETECTOR_H
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>
#include <QMap>

#include "cnn_data_type.h"

class BaseDetector
{
public:
    BaseDetector();
    virtual ~BaseDetector();

    virtual int initModel(const std::string modelNet, const std::string modelWeight="") = 0;
    virtual int initDetectorParameters(const int dataWidth, const int dataHeight,
                                       const float confidenceThreshold, const QMap<int, QString> &labels) = 0;
    virtual void processDetect(const cv::Mat &inputImage, std::vector<Detect2dBox> &objectRect) = 0;

    virtual std::string getLabelName(const int indice);

protected:

    std::vector<int> labelIds;
    std::vector<std::string> labelNames;

private:
    virtual void saveConfig() = 0;
    virtual void loadConfig() = 0;
};

#endif // CAFFEDETECTOR_H
