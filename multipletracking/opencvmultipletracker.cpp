#include "opencvmultipletracker.h"

OpencvMultipletracker::OpencvMultipletracker(std::string trackingMethod): trackingMethod(trackingMethod)
{
    init();
}

OpencvMultipletracker::~OpencvMultipletracker()
{
    trajectorys.clear();
}

void OpencvMultipletracker::mutilpleTracking(const cv::Mat& preFrame, const cv::Mat& inFrame, const std::vector<TrackingObject>& objects)
{

}

void OpencvMultipletracker::getTrackingResult(std::vector<TrackingObject>& result)
{

}

void OpencvMultipletracker::initTracking()
{

}

void OpencvMultipletracker::init()
{
    trajectorys.clear();
    trackingObjects.clear();
}

void OpencvMultipletracker::createTrackerByName(std::string name)
{

}

