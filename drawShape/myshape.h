#ifndef MYSHAPE_H
#define MYSHAPE_H

#include <QObject>
#include <QString>
#include <QMap>

#define EDGE_TOOL 0

typedef enum ShapeType{
    UNSHAPE = -1,
    RECT_SHAPE = 0,
    LINE_SHAPE = 1,
    POLYGON_SHAPE = 2,
    POLYLINE_SHAPE = 3,
    RECT_TRACK_SHAPE = 4,
    POLYGON_SEGMENT_SHAPE = 5,
    LANE_SEGMENT_SHAPE = 6,
    INSTANCE_SEGMENT_SHAPE = 7,
    OCR_POLYGON_SHAPE = 8,
    MAX_IMAGE_SHAPE_TYPE = 9,
    RECT3D_SHAPE = 10,
    MAX_SHAPE_TYPE = 11
}ShapeType;

class MyShape
{
public:
    MyShape();
    ~MyShape();

    QString getShapeName(int shapeId);
    QMap<int, QString> getAllShape() const;
    QMap<int, QString> getImageShape() const;
    QMap<int, QString> getSegmentShape() const;
    QMap<int, QString> getOCRShape() const;
    QMap<int, QString> getTracking2dShape() const;

private:

    QMap<int, QString> shape;

private:

    void init();

};

#endif // MYSHAPE_H
