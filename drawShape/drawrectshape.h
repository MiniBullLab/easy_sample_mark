#ifndef DRAWRECTSHAPE_H
#define DRAWRECTSHAPE_H

#include "drawshape.h"

class DrawRectShape : public DrawShape
{
    Q_OBJECT
public:
    DrawRectShape(MarkDataType dataType, QObject *parent = 0);
    ~DrawRectShape();

    void initDraw();

    //rect
    int drawMousePress(const QPoint point, bool &isDraw);
    int drawMouseMove(const QPoint point, bool &isDraw);
    int drawMouseRelease(QWidget *parent, const QPoint point, bool &isDraw);
    void removeShape(bool &isDraw);
    bool isInShape(const QPoint &point);

    void drawPixmap(const ShapeType shapeID, QPainter &painter);

    void setObjectList(QList<MyObject> list);
    void getObjectList(QList<MyObject> &list);

signals:

public slots:

private:

    void getCurrentRect(QRect &rect, bool &isDraw);
    QList<QPoint> getRectListPoints(const QString sampleClass);
    int nearRectPiont(const QPoint point);
    void updateRect(const QPoint point);

private:

    int nearRectIndex;
    int rectPointIndex;
    int removeRectIndex;
    QRect currentRect;
    QList<MyObject> listRect;
};

#endif // DRAWRECTSHAPE_H
