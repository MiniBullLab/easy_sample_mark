#ifndef DRAWINSTANCESHAPE_H
#define DRAWINSTANCESHAPE_H

#include "drawShape/drawshape.h"

class DrawInstanceShape : public DrawShape
{
    Q_OBJECT
public:
    DrawInstanceShape(MarkDataType dataType, bool isSegment, QObject *parent = 0);
    ~DrawInstanceShape();

    void initDraw();

    //rect
    int drawMousePress(const QPoint point, bool &isDraw);
    int drawMouseMove(const QPoint point, bool &isDraw);
    int drawMouseRelease(QWidget *parent, const QPoint point, bool &isDraw);
    void removeShape(bool &isDraw);
    bool isInShape(const QPoint &point);

    void cancelDrawShape(bool &isDraw) override;

    void drawPixmap(const ShapeType shapeID, QPainter &painter);

    void setObjectList(QList<MyObject> list);
    void getObjectList(QList<MyObject> &list);

signals:

public slots:

private:

    QList<QPoint> getRectListPoints(const QString sampleClass);
    int nearRectPiont(const QPoint point);
    void updateRect(const QPoint point);

    int nearPolygonPoint(const QPoint point);
    void updatePolygon(const QPoint point);

private:
    bool finishDrawRect;
    bool finishDrawPolygon;
    bool nearFirstPoint;

    int nearPolygonIndex;
    int polygonPointIndex;
    QPoint firstPoint;
    QPolygon currentPolygon;

    int nearRectIndex;
    int rectPointIndex;
    int removeRectIndex;
    QRect currentRect;
    QList<MyObject> listInstance;

    bool isSegment;
};

#endif // DRAWINSTANCESHAPE_H
