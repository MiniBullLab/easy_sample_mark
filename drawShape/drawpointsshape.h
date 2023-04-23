#ifndef DRAWPOINTSSHAPE_H
#define DRAWPOINTSSHAPE_H

#include "drawshape.h"

class DrawPointsShape : public DrawShape
{
    Q_OBJECT
public:
    DrawPointsShape(MarkDataType dataType,
                  QObject *parent = 0);
    ~DrawPointsShape() override;

    void initDraw() override;

    int drawMousePress(const QPoint point, bool &isDraw) override;
    int drawMouseMove(const QPoint point, bool &isDraw) override;
    int drawMouseRelease(QWidget *parent, const QPoint point, bool &isDraw) override;

    int drawMouseDoubleClick(QWidget *parent, const QPoint point, bool &isDraw) override;

    void removeShape(bool &isDraw) override;
    bool isInShape(const QPoint &point) override;

    void drawPixmap(const ShapeType shapeID, QPainter &painter) override;

    void setObjectList(QList<MyObject> list) override;
    void getObjectList(QList<MyObject> &list) override;

    int getObjectSize() override;

signals:

public slots:

private:
    int nearObjectPoint(const QPoint point);
    void updateObject(const QPoint point);

    void drawMark(const QList<QPoint> &pointList, QPainter &painter);

private:

    bool nearFirstPoint;

    int nearObjectIndex;
    int objectPointIndex;
    int removeObjectIndex;
    QPoint firstPoint;
    QList<QPoint> pointsList;
    QList<MyObject> objectList;
};

#endif // DRAWPOINTSSHAPE_H
