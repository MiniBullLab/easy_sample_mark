#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "imagelabel.h"
#include <QPaintEvent>
#include <QPainter>

ImageLabel::ImageLabel(QWidget *parent): QLabel(parent)
{
    pointList.clear();
    myDrawCursor = QCursor(QPixmap(tr(":/images/images/cross.png")));
    this->setMouseTracking(true);
    this->setCursor(myDrawCursor);
    this->zoomValue = 100;
}

ImageLabel::~ImageLabel()
{

}

void ImageLabel::setNewQImage(QImage &image)
{
    pointList.clear();
    this->zoomValue = 100;
    tempPixmap = QPixmap::fromImage(image.copy());
    this->update();
}

void ImageLabel::setScaleImage(const int value)
{
    this->zoomValue = value;
    this->update();
}

QList<QPoint> ImageLabel::getPointList() const
{
    return this->pointList;
}

void ImageLabel::mousePressEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }

    if(e->button() == Qt::LeftButton)
    {

    }
    QLabel::mousePressEvent(e);
}

void ImageLabel::mouseMoveEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }
    QLabel::mouseMoveEvent(e);
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }

    if(e->button() == Qt::LeftButton)
    {
        QPoint point = e->pos();
        QPainter painter;
        painter.begin(&tempPixmap);
        painter.setBrush(QColor("#3CFF55"));
        painter.drawEllipse(point, 2, 2);
        painter.end();
        pointList.append(point);
        this->update();
        emit signalSelectPoint(point);
    }
    QLabel::mouseReleaseEvent(e);
}

void ImageLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {

    }
    QLabel::mouseDoubleClickEvent(event);
}

void ImageLabel::paintEvent(QPaintEvent *e)
{
    if(!tempPixmap.isNull())
    {
        QPainter painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        float scale = this->zoomValue / 100.0f;
        painter.scale(scale, scale);
        painter.translate(this->offsetToCenter());
        painter.drawPixmap(QPoint(0,0), tempPixmap);
        painter.end();
        this->resize(tempPixmap.width() * scale, tempPixmap.height() * scale);
        this->setAutoFillBackground(true);
    }
    QLabel::paintEvent(e);
}

QPointF ImageLabel::offsetToCenter()
{
    QSize area = this->size();
    float scale = this->zoomValue / 100.0f;
    float w = this->tempPixmap.width() * scale;
    float h = this->tempPixmap.height() * scale;
    float aw = area.width();
    float ah = area.height();
    float x = 0;
    float y = 0;
    if(aw > w)
    {
        x = (aw - w) / (2 * scale);
    }
    if(ah > h)
    {
        y = (ah - h) / (2 * scale);
    }
    return QPointF(x, y);
}
