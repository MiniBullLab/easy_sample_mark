#ifndef CNN_DATA_TYPE_H
#define CNN_DATA_TYPE_H

#include <string>

typedef struct Detect2dBox {
    Detect2dBox(float x1=0, float y1=0, float x2=0, float y2=0,
            float confidence=0, float classID=-1, float objectID=-1) {
        this->minX = x1;
        this->minY = y1;
        this->maxX = x2;
        this->maxY = y2;
        this->confidence = confidence;
        this->classID = classID;
        this->objectID = objectID;
        this->objectName = "";
    }
    float minX;
    float minY;
    float maxX;
    float maxY;
    float confidence;
    float classID;
    float objectID;
    std::string objectName;
} Detect2dBox;

#endif // CNN_DATA_TYPE_H
