#define _DLL_EXPORTS

#include "deepsort.h"
#include <iostream>

DeepSort::DeepSort(std::string modelPath, int batchSize, int featureDim) {
    this->enginePath = modelPath;
    this->batchSize = batchSize;
    this->featureDim = featureDim;
    this->imgShape = cv::Size(64, 128);
    this->maxBudget = 100;
    this->maxCosineDist = 0.2;
    init();
}

DeepSort::~DeepSort() {
    delete objTracker;
}

void DeepSort::sort(cv::Mat& frame, vector<Detect2dBox>& dets) {
    // preprocess Mat -> DETECTION
    DETECTIONS detections;
    vector<CLSCONF> clsConf;

    for (auto i : dets) {
        DETECTBOX box(i.minX, i.minY, i.maxX - i.minX, i.maxY - i.minY);
        DETECTION_ROW d;
        d.tlwh = box;
        d.confidence = i.confidence;
        detections.push_back(d);
        clsConf.push_back(CLSCONF((int)i.classID, i.confidence));
    }
    result.clear();
    results.clear();
    if (detections.size() > 0) {
        DETECTIONSV2 detectionsv2 = make_pair(clsConf, detections);
        sort(frame, detectionsv2);
    }
    // postprocess DETECTION -> Mat
    dets.clear();
    for (auto r : result) {
        DETECTBOX i = r.second;
        Detect2dBox b(i(0), i(1), i(2) + i(0), i(3) + i(1), 1.);
        b.objectID = (float)r.first;
        dets.push_back(b);
    }
    for (int i = 0; i < results.size(); ++i) {
        CLSCONF c = results[i].first;
        dets[i].classID = c.cls;
        dets[i].confidence = c.conf;
    }
}

void DeepSort::sort(vector<Detect2dBox>& dets) {
    DETECTIONS detections;
    for (Detect2dBox i : dets) {
        DETECTBOX box(i.minX, i.minY, i.maxX - i.minX, i.maxY - i.minY);
        DETECTION_ROW d;
        d.tlwh = box;
        d.confidence = i.confidence;
        detections.push_back(d);
    }
    if (detections.size() > 0)
        sort(detections);
    dets.clear();
    for (auto r : result) {
        DETECTBOX i = r.second;
        Detect2dBox b(i(0), i(1), i(2), i(3), 1.);
        b.objectID = r.first;
        dets.push_back(b);
    }
}


void DeepSort::sort(cv::Mat& frame, DETECTIONS& detections) {
    bool flag = featureExtractor->getRectsFeature(frame, detections);
    if (flag) {
        objTracker->predict();
        objTracker->update(detections);
        //result.clear();
        for (Track& track : objTracker->tracks) {
            if (!track.is_confirmed() || track.time_since_update > 1)
                continue;
            result.push_back(make_pair(track.track_id, track.to_tlwh()));
        }
    }
}

void DeepSort::sort(cv::Mat& frame, DETECTIONSV2& detectionsv2) {
    std::vector<CLSCONF>& clsConf = detectionsv2.first;
    DETECTIONS& detections = detectionsv2.second;
#ifdef FEATURE_MATCH_EN
    bool flag = featureExtractor->getRectsFeature(frame, detections);
#else
    bool flag = 1;
#endif
    std::cout << "[deepsort] Extract REID feature!" << std::endl;
    if (flag) {
        std::cout << "[deepsort] Start tracking!" << std::endl;
        objTracker->predict();
        std::cout << "[deepsort] Predict track object done!" << std::endl;
        objTracker->update(detectionsv2);
        std::cout << "[deepsort] Update object track id done!" << std::endl;
        result.clear();
        results.clear();
        for (Track& track : objTracker->tracks) {
            if (!track.is_confirmed() || track.time_since_update > 1)
                continue;
            result.push_back(make_pair(track.track_id, track.to_tlwh()));
            results.push_back(make_pair(CLSCONF(track.cls, track.conf), track.to_tlwh()));
        }
        std::cout << "[deepsort] Track object has been record in results done!" << std::endl;
    }
}

void DeepSort::sort(DETECTIONS& detections) {
    bool flag = featureExtractor->getRectsFeature(detections);
    if (flag) {
        objTracker->predict();
        objTracker->update(detections);
        result.clear();
        for (Track& track : objTracker->tracks) {
            if (!track.is_confirmed() || track.time_since_update > 1)
                continue;
            result.push_back(make_pair(track.track_id, track.to_tlwh()));
        }
    }
}

void DeepSort::init() {
    objTracker = new tracker(maxCosineDist, maxBudget);
    featureExtractor = new FeatureTensor(batchSize, imgShape, featureDim, 0);

    featureExtractor->loadOnnx(enginePath);
}