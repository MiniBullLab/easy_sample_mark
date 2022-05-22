#ifndef FEATURETENSOR_H
#define FEATURETENSOR_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include "model.hpp"
#include "datatype.h"

class FeatureTensor {
public:
    FeatureTensor(const int maxBatchSize, const cv::Size imgShape, const int featureDim, int gpuID);

    ~FeatureTensor();

public:
    bool getRectsFeature(const cv::Mat& img, DETECTIONS& det);
    bool getRectsFeature(DETECTIONS& det);
    void loadOnnx(std::string onnxPath);
    //int getResult(float*& buffer);
    cv::Mat doInference(std::vector<cv::Mat>& imgMats);

private:
    cv::Mat doInference_run(std::vector<cv::Mat> imgMats);
    void stream2det(cv::Mat stream, DETECTIONS& det);

private:

    cv::dnn::Net dnn_engine;
    const int maxBatchSize;
    const cv::Size imgShape;
    const int featureDim;

private:
    int curBatchSize;

    bool initFlag;
    cv::Mat outputBuffer;

    int inputIndex, outputIndex;
    void* buffers[2];

    // BGR format
    float means[3], std[3];
    const std::string inputName, outputName;
};

#endif
