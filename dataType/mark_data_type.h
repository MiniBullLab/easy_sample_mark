#ifndef MARK_DATA_TYPE_H
#define MARK_DATA_TYPE_H

typedef enum MarkDataType{
    UNKNOWN = 0,
    IMAGE = 1,
    VIDEO = 2,
    SEGMENT = 3,
    OCR = 4,
    IMAGE_TRACKING = 5,
    PCD = 6,
    MAX_CONUT
}MarkDataType;

#endif // MARK_DATA_TYPE_H
