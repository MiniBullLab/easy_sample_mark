#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QCursor>

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    ImageLabel(QWidget *parent = 0);
    ~ImageLabel();

     void setNewQImage(QImage &image);
     void setScaleImage(const int value);

     QList<QPoint> getPointList() const;

signals:
    void signalSelectPoint(QPoint point);

protected:
     void mouseMoveEvent(QMouseEvent *e) override;
     void mousePressEvent(QMouseEvent *e) override;
     void mouseReleaseEvent(QMouseEvent *e) override;
     void mouseDoubleClickEvent(QMouseEvent *event) override;
     void paintEvent(QPaintEvent *e) override;

private:
    QPixmap tempPixmap;

    QList<QPoint> pointList;

    QCursor myDrawCursor;

    int zoomValue;

    QPointF offsetToCenter();
};

#endif // IMAGELABEL_H
