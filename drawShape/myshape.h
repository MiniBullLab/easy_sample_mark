﻿#ifndef MYSHAPE_H
#define MYSHAPE_H

#include <QObject>
#include <QString>
#include <QMap>

typedef enum ShapeType{
    UNSHAPE = -1,
    RECT_SHAPE = 0,
    LINE_SHAPE = 1,
    POLYGON_SHAPE = 2
}ShapeType;

class MyShape
{
public:
    MyShape();
    ~MyShape();

    QString getShapeName(int shapeId);
    QMap<int, QString> getAllShape() const;

private:

    QMap<int, QString> shape;

private:

    void init();

};

#endif // MYSHAPE_H
