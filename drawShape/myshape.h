#ifndef MYSHAPE_H
#define MYSHAPE_H

#include <QObject>
#include <QString>
#include <QMap>

#define EDGE_TOOL 1

typedef enum ShapeType{
    UNSHAPE = -1,
    RECT_SHAPE = 0,
    LINE_SHAPE = 1,
    POLYGON_SHAPE = 2,
    POLYLINE_SHAPE = 3,
    POLYGON_SEGMENT_SHAPE = 4,
    LANE_SEGMENT_SHAPE = 5,
    INSTANCE_SEGMENT_SHAPE = 6,
    MAX_IMAGE_SHAPE_TYPE = 7,
    RECT3D_SHAPE = 8,
    MAX_SHAPE_TYPE = 9
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

private:

    QMap<int, QString> shape;

private:

    void init();

};

#endif // MYSHAPE_H
