#ifndef TRACKING2DLABEL_H
#define TRACKING2DLABEL_H

#include "drawWidget/imagedrawlabel.h"

#include <QList>
#include <QMap>

#include "drawShape/drawrecttrackshape.h"
#include "dataType/mark_data_type.h"

class Tracking2DLabel : public ImageDrawLabel
{
    Q_OBJECT

public:
    Tracking2DLabel(QWidget *parent = 0);
    ~Tracking2DLabel();

    void clearDraw() override;
    void setNewQImage(QImage &image) override;
    void setDrawShape(int shapeID) override;
    void resetDraw() override;

public slots:

protected:
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent * event) override;

    void paintEvent(QPaintEvent *e) override;

    void contextMenuEvent(QContextMenuEvent * event) override;

    void setDrawShapeObjects() override;
    void drawPixmap() override;

private:

    QPointF offsetToCenter();
    QPoint scalePoint(const QPoint point);

    void initData();

private:
    int zoomValue;
};

#endif // TRACKING2DLABEL_H
