#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "tracking2dlabel.h"
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include "sampleMarkParam/manualparamterconfig.h"

Tracking2DLabel::Tracking2DLabel(QWidget *parent):
    ImageDrawLabel(parent)
{
    initData();
}

Tracking2DLabel::~Tracking2DLabel()
{
    QMap<ShapeType, DrawShape*>::const_iterator drawIterator;
    for(drawIterator = drawList.constBegin(); drawIterator != drawList.constEnd(); ++drawIterator)
    {
        if(drawList[drawIterator.key()] != NULL)
        {
            delete drawList[drawIterator.key()];
            drawList[drawIterator.key()] = NULL;
        }
    }
}

void Tracking2DLabel::clearDraw()
{
    QMap<ShapeType, DrawShape*>::const_iterator drawIterator;
    for(drawIterator = drawList.constBegin(); drawIterator != drawList.constEnd(); ++drawIterator)
    {
        drawList[drawIterator.key()]->initDraw();
    }
    drawPixmap();
}

void Tracking2DLabel::setNewQImage(QImage &image)
{
    mp = QPixmap::fromImage(image);
    this->zoomValue = 100;
    drawPixmap();
}

void Tracking2DLabel::setDrawShape(int shapeID)
{
    if(shapeID >= 0 && shapeID < ShapeType::MAX_IMAGE_SHAPE_TYPE)
    {
        this->shapeType = static_cast<ShapeType>(shapeID);
        drawPixmap();
    }
    else
    {
        QMessageBox::information(this, tr("标注形状"), tr("选择的标注形状有误！"));
    }
}

void Tracking2DLabel::resetDraw()
{
    this->zoomValue = 100;
    drawPixmap();
}

void Tracking2DLabel::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu* popMenu = new QMenu(this);
    QPoint point = this->mapFromGlobal(QCursor::pos());
    bool isFind = drawList[this->shapeType]->isInShape(scalePoint(point));
    if(isFind)
    {
        popMenu->addAction(removeRectAction);
    }
    popMenu->exec(QCursor::pos());
    QLabel::contextMenuEvent(event);
}

void Tracking2DLabel::mousePressEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }

    if(e->button() == Qt::LeftButton)
    {
        bool isDraw = false;
        QPoint point = e->pos();
        int mouseChange = drawList[this->shapeType]->drawMousePress(scalePoint(point), isDraw);
        if(mouseChange == 1)
        {
            this->setCursor(Qt::CrossCursor);
        }
        else if(mouseChange == 2)
        {
            this->setCursor(Qt::SizeAllCursor);
        }
        if(isDraw)
        {
            drawPixmap();
        }
    }
    QLabel::mousePressEvent(e);
}

void Tracking2DLabel::mouseMoveEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }
    bool isDraw = false;
    QPoint point = e->pos();
    int mouseChange = drawList[this->shapeType]->drawMouseMove(scalePoint(point), isDraw);
    if(mouseChange == 1)
    {
        this->setCursor(Qt::CrossCursor);
    }
    else if(mouseChange == 2)
    {
        this->setCursor(Qt::SizeAllCursor);
    }
    if(isDraw)
    {
        drawPixmap();
    }
    QLabel::mouseMoveEvent(e);
}

void Tracking2DLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }

    if(e->button() == Qt::LeftButton)
    {
        bool isDraw = false;
        QPoint point = e->pos();
        drawList[this->shapeType]->setVisibleSampleClass(this->sampleClass);
        drawList[this->shapeType]->drawMouseRelease(this, scalePoint(point), isDraw);
        if(isDraw)
        {
            drawPixmap();
        }
    }
    QLabel::mouseReleaseEvent(e);
}

void Tracking2DLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        bool isDraw = false;
        QPoint point = event->pos();
        drawList[this->shapeType]->setVisibleSampleClass(this->sampleClass);
        drawList[this->shapeType]->drawMouseDoubleClick(this, scalePoint(point), isDraw);
        if(isDraw)
        {
            drawPixmap();
        }
    }
    //QLabel::mouseDoubleClickEvent(event);
}

void Tracking2DLabel::wheelEvent(QWheelEvent * event)
{
    if(event->delta() > 0)
    {
        this->zoomValue++;
        if(this->zoomValue > ManualParamterConfig::getMaxScale())
        {
            this->zoomValue = ManualParamterConfig::getMaxScale();
        }
    }
    else
    {
        this->zoomValue--;
        if(this->zoomValue < ManualParamterConfig::getMinSacle())
        {
            this->zoomValue = ManualParamterConfig::getMinSacle();
        }
    }
    drawPixmap();
    QWidget::wheelEvent(event);
}

void Tracking2DLabel::paintEvent(QPaintEvent *e)
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

void Tracking2DLabel::setDrawShapeObjects()
{
    drawList[ShapeType::RECT_TRACK_SHAPE]->setObjectList(rectObejcts);
    drawPixmap();
}

void Tracking2DLabel::drawPixmap()
{
    if(!mp.isNull())
    {
        QPainter painter;
        tempPixmap = mp.copy();
        painter.begin(&tempPixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.setPen(QPen(QColor("#3CFF55")));
        QMap<ShapeType, DrawShape*>::const_iterator drawIterator;
        for(drawIterator = drawList.constBegin(); drawIterator != drawList.constEnd(); ++drawIterator)
        {
            painter.save();
            drawList[drawIterator.key()]->setVisibleSampleClass(this->sampleClass);
            drawList[drawIterator.key()]->drawPixmap(this->shapeType, painter);
            painter.restore();
        }
        painter.end();
    }
    this->update();
}

QPointF Tracking2DLabel::offsetToCenter()
{
    QSize area = this->size();
    float scale = this->zoomValue / 100.0f;
    float w = this->mp.width() * scale;
    float h = this->mp.height() * scale;
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

QPoint Tracking2DLabel::scalePoint(const QPoint point)
{
    float scale = this->zoomValue / 100.0f;
    QPoint resultPoint(static_cast<int>(point.x() / scale),
                       static_cast<int>(point.y() / scale));
    return resultPoint;
}

void Tracking2DLabel::initData()
{
    this->setMouseTracking(true);
    this->setCursor(Qt::CrossCursor);

    this->shapeType = ShapeType::RECT_TRACK_SHAPE;

    this->zoomValue = 100;

    drawList.clear();
    drawList.insert(ShapeType::RECT_TRACK_SHAPE, new DrawRectTrackShape(MarkDataType::IMAGE_TRACKING));
    QMap<ShapeType, DrawShape*>::const_iterator drawIterator;
    for(drawIterator = drawList.constBegin(); drawIterator != drawList.constEnd(); ++drawIterator)
    {
        drawList[drawIterator.key()]->setVisibleSampleClass(this->sampleClass);
    }
}
