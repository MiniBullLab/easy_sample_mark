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

     QList<QPoint> getPointList() const;

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

    QPointF offsetToCenter();
};

#endif // IMAGELABEL_H
