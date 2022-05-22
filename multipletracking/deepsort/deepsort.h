//#ifndef DEEPSORT_H
//#define DEEPSORT_H

#include <opencv2/core.hpp>
#include "deepLearning/cnn_data_type.h"

#include "featuretensor.h"
#include "tracker.h"
#include <vector>

class DeepSort {

public:    

    DeepSort(std::string modelPath, int batchSize, int featureDim);

    ~DeepSort();

    void sort(cv::Mat& frame, std::vector<Detect2dBox>& dets);
    void sort(std::vector<Detect2dBox>& dets);
     
private:
    void sort(cv::Mat& frame, DETECTIONS& detections);
    void sort(cv::Mat& frame, DETECTIONSV2& detectionsv2);    

    void sort(DETECTIONS& detections);

    void init();

private:
    std::string enginePath;
    int batchSize;
    int featureDim;
    cv::Size imgShape;
    float confThres;
    float nmsThres;
    int maxBudget;
    float maxCosineDist;

private:
    std::vector<RESULT_DATA> result;
    vector<std::pair<CLSCONF, DETECTBOX>> results;
    tracker* objTracker;
    FeatureTensor* featureExtractor;
};

//#endif  //deepsort.h
