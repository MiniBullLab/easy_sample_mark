#ifndef OCRLABEL_H
#define OCRLABEL_H

#include "drawWidget/imagedrawlabel.h"

#include <QList>
#include <QCursor>

#include "drawShape/drawpolygonshape.h"

class OCRLabel : public ImageDrawLabel
{
    Q_OBJECT

public:
    OCRLabel(QWidget *parent = 0);
    ~OCRLabel() override;

    void clearDraw() override;
    void setNewQImage(QImage &image) override;
    void setDrawShape(int shapeID) override;
    void resetDraw() override;

public slots:

    void slotEditObject();

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
    float scale;
    QCursor myDrawCursor;
};

#endif // SEGMENTLABEL_H
