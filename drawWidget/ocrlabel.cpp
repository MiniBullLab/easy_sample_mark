#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "ocrlabel.h"
#include <QMenu>
#include <QMessageBox>
#include <QDebug>
#include "sampleMarkParam/manualparamterconfig.h"

OCRLabel::OCRLabel(QWidget *parent):
    ImageDrawLabel(parent)
{
    initData();
}

OCRLabel::~OCRLabel()
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

void OCRLabel::clearDraw()
{
    QMap<ShapeType, DrawShape*>::const_iterator drawIterator;
    for(drawIterator = drawList.constBegin(); drawIterator != drawList.constEnd(); ++drawIterator)
    {
        drawList[drawIterator.key()]->initDraw();
    }
    drawPixmap();
}

void OCRLabel::setNewQImage(QImage &image)
{
    mp = QPixmap::fromImage(image);
    this->scale = 1.0f;
    drawPixmap();
}

void OCRLabel::setDrawShape(int shapeID)
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

void OCRLabel::resetDraw()
{
    this->scale = 1.0f;
    drawPixmap();
}

void OCRLabel::slotEditObject()
{
    bool isDraw = false;
    if(drawList.count() > 0)
    {
        drawList[this->shapeType]->editMark(isDraw);
    }
    if(isDraw)
    {
        drawPixmap();
    }
}

void OCRLabel::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu* popMenu = new QMenu(this);
    QPoint point = this->mapFromGlobal(QCursor::pos());
    bool isFind = drawList[this->shapeType]->isInShape(scalePoint(point));
    if(isFind)
    {
        popMenu->addAction(editRectAction);
        popMenu->addAction(removeRectAction);
    }

    popMenu->exec(QCursor::pos());
    QLabel::contextMenuEvent(event);
}

void OCRLabel::mousePressEvent(QMouseEvent *e)
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
            this->setCursor(myDrawCursor);
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
}

void OCRLabel::mouseMoveEvent(QMouseEvent *e)
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
        this->setCursor(myDrawCursor);
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

void OCRLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if(!this->rect().contains(e->pos()))
    {
        return;
    }

    if(e->button() == Qt::LeftButton)
    {
        bool isDraw = false;
        QPoint point = e->pos();
        drawList[this->shapeType]->drawMouseRelease(this, scalePoint(point), isDraw);
        if(isDraw)
        {
            drawPixmap();
        }
    }
}

void OCRLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        bool isDraw = false;
        QPoint point = event->pos();
        drawList[this->shapeType]->drawMouseDoubleClick(this, scalePoint(point), isDraw);
        if(isDraw)
        {
            drawPixmap();
        }
    }
    //QLabel::mouseDoubleClickEvent(event);
}

void OCRLabel::wheelEvent(QWheelEvent * event)
{
    int value = event->delta();
    float scaleRatio = value / (8 * 360.0f);
    scale += scaleRatio;
    if(scale > ManualParamterConfig::getMaxScale() / 100.0f)
    {
        scale = ManualParamterConfig::getMaxScale() / 100.0f;
    }
    else if(scale < ManualParamterConfig::getMinSacle() / 100.0f)
    {
        scale = ManualParamterConfig::getMinSacle() / 100.0f;
    }
    drawPixmap();
    QLabel::wheelEvent(event);
}

void OCRLabel::paintEvent(QPaintEvent *e)
{
    if(!tempPixmap.isNull())
    {
        QPainter painter;
        painter.begin(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.scale(this->scale, this->scale);
        painter.translate(this->offsetToCenter());
        painter.drawPixmap(QPoint(0,0), tempPixmap);
        painter.end();
        this->resize(tempPixmap.width() * this->scale, tempPixmap.height() * this->scale);
        this->setAutoFillBackground(true);
    }
    QLabel::paintEvent(e);
}

void OCRLabel::setDrawShapeObjects()
{
    drawList[ShapeType::OCR_POLYGON_SHAPE]->setObjectList(ocrObejcts);
    drawPixmap();
}

void OCRLabel::drawPixmap()
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
            drawList[drawIterator.key()]->drawPixmap(this->shapeType, painter);
            painter.restore();
        }
        painter.end();
    }
    this->update();
}

QPointF OCRLabel::offsetToCenter()
{
    QSize area = this->size();
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

QPoint OCRLabel::scalePoint(const QPoint point)
{
    QPoint resultPoint(static_cast<int>(point.x() / scale),
                       static_cast<int>(point.y() / scale));
    return resultPoint;
}

void OCRLabel::initData()
{
    myDrawCursor = QCursor(QPixmap(tr(":/images/images/cross.png")));
    this->setMouseTracking(true);
    this->setCursor(myDrawCursor);

    this->scale = 1.0f;

    this->shapeType = ShapeType::OCR_POLYGON_SHAPE;
    drawList.clear();
    drawList.insert(ShapeType::OCR_POLYGON_SHAPE,
                    new DrawPolygonShape(MarkDataType::OCR, false));
    QMap<ShapeType, DrawShape*>::const_iterator drawIterator;
    for(drawIterator = drawList.constBegin(); drawIterator != drawList.constEnd(); ++drawIterator)
    {
        drawList[drawIterator.key()]->setVisibleSampleClass(this->sampleClass);
    }

    connect(editRectAction, &QAction::triggered, this, &OCRLabel::slotEditObject);
}
