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
