#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "myshape.h"

MyShape::MyShape()
{
    init();
}

MyShape::~MyShape()
{

}

QString MyShape::getShapeName(int shapeId)
{
    if(this->shape.contains(shapeId))
    {
        return this->shape[shapeId];
    }
    else
    {
        return "";
    }
}

QMap<int, QString> MyShape::getAllShape() const
{
    return this->shape;
}

QMap<int, QString> MyShape::getImageShape() const
{
    QMap<int, QString> result;
    result.clear();
    result.insert(ShapeType::RECT_SHAPE, this->shape[ShapeType::RECT_SHAPE]);
#if EDGE_TOOL == 0
    result.insert(ShapeType::LINE_SHAPE, this->shape[ShapeType::LINE_SHAPE]);
    result.insert(ShapeType::POLYGON_SHAPE, this->shape[ShapeType::POLYGON_SHAPE]);
    result.insert(ShapeType::POLYLINE_SHAPE, this->shape[ShapeType::POLYLINE_SHAPE]);
#endif
    return result;
}

QMap<int, QString> MyShape::getSegmentShape() const
{
    QMap<int, QString> result;
    result.clear();
    result.insert(ShapeType::POLYGON_SEGMENT_SHAPE, QObject::tr("多边形分割"));
#if EDGE_TOOL == 0
    result.insert(ShapeType::LANE_SEGMENT_SHAPE, QObject::tr("折线分割"));
    result.insert(ShapeType::INSTANCE_SEGMENT_SHAPE, this->shape[ShapeType::INSTANCE_SEGMENT_SHAPE]);
#endif
    return result;
}

QMap<int, QString> MyShape::getOCRShape() const
{
    QMap<int, QString> result;
    result.clear();
    result.insert(ShapeType::OCR_POLYGON_SHAPE, QObject::tr("多边形标注"));
    return result;
}

QMap<int, QString> MyShape::getTracking2dShape() const
{
    QMap<int, QString> result;
    result.clear();
    result.insert(ShapeType::RECT_TRACK_SHAPE, this->shape[ShapeType::RECT_TRACK_SHAPE]);
    return result;
}

void MyShape::init()
{
    shape.clear();
    shape.insert(ShapeType::RECT_SHAPE, QObject::tr("矩形"));
    shape.insert(ShapeType::LINE_SHAPE, QObject::tr("直线"));
    shape.insert(ShapeType::POLYGON_SHAPE, QObject::tr("多边形"));
    shape.insert(ShapeType::POLYLINE_SHAPE, QObject::tr("折线"));
    shape.insert(ShapeType::RECT_TRACK_SHAPE, QObject::tr("矩形追踪"));
    shape.insert(ShapeType::INSTANCE_SEGMENT_SHAPE, QObject::tr("实例分割"));
}
