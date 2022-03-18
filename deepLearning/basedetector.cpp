#include "basedetector.h"
#include <iostream>

BaseDetector::BaseDetector()
{

}

BaseDetector::~BaseDetector()
{

}

std::string BaseDetector::getLabelName(const int indice)
{
    std::string labelName = "";
    for(size_t index = 0; index < labelIds.size(); index++)
    {
        if(labelIds[index] == indice)
        {
            labelName = labelNames[index];
            break;
        }
    }
    return labelName;
}


void BaseDetector::showDetection(cv::Mat &image, std::vector<Detect2dBox> &objectRect)
{
    if(image.empty())
    {
        return;
    }

    int objectColor = 10;
    for(size_t index = 0; index < objectRect.size(); index++)
    {
        cv::Rect tempRect(cv::Point(objectRect[index].minX, objectRect[index].minY),
                          cv::Point(objectRect[index].maxX, objectRect[index].maxY));
        cv::rectangle(image, tempRect, cv::Scalar(255 * (objectColor / 100), 255 * ((objectColor / 10) % 10), 255 * (objectColor % 10)));
        cv::putText(image, objectRect[index].objectName, tempRect.tl(), cv::FONT_HERSHEY_COMPLEX, 0.5,
                    cv::Scalar(255 * (objectColor / 100), 255 * ((objectColor / 10) % 10), 255 * (objectColor % 10)));
    }
}
