#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "drawpointsshape.h"
#include <QMessageBox>
#include "sampleMarkParam/manualparamterconfig.h"
#include "sampleMarkParam/segmentparamterconfig.h"
#include "selectMarkInfo/selectmarkclasswindow.h"

DrawPointsShape::DrawPointsShape(MarkDataType dataType,
                                QObject *parent) :
    DrawShape(dataType, parent)
{
    initDraw();
}

DrawPointsShape::~DrawPointsShape()
{
}

void DrawPointsShape::initDraw()
{
    drawMousePressed = false;
    moveMousePressed = false;

    nearFirstPoint = false;
    firstPoint = QPoint(-1, -1);

    nearObjectIndex = -1;
    objectPointIndex = 0;
    removeObjectIndex = -1;
    pointsList.clear();
    objectList.clear();
}

int DrawPointsShape::drawMousePress(const QPoint point, bool &isDraw)
{
    int mouseChange = 0;
    if(nearObjectIndex >= 0)
    {
        mouseChange = 2;
        drawMousePressed = false;
        moveMousePressed = true;
        updateObject(point);
    }
    else
    {
        if(firstPoint.x() < 0 || firstPoint.y() < 0)
        {
            firstPoint = point;
        }
        if(!nearFirstPoint)
        {
            pointsList.append(point);
        }
        mouseChange = 1;
        moveMousePressed = false;
        drawMousePressed = true;
    }
    isDraw = true;
    return mouseChange;
}

int DrawPointsShape::drawMouseMove(const QPoint point, bool &isDraw)
{
    int mouseChange = 0;
    isDraw = false;
    if(moveMousePressed)
    {
        updateObject(point);
        mouseChange = 2;
        isDraw = true;
    }
    else if(!drawMousePressed)
    {
        nearObjectIndex = nearObjectPoint(point);
        if(nearObjectIndex >= 0 || nearFirstPoint)
        {
            mouseChange = 2;
        }
        else
        {
            mouseChange = 1;
        }
    }
    return mouseChange;
}

int DrawPointsShape::drawMouseRelease(QWidget *parent, const QPoint point, bool &isDraw)
{
    if(moveMousePressed)
    {
        QList<QPoint> polygon = objectList[nearObjectIndex].getPointList();
        int pointCount = polygon.count();
        int preIndex = ((objectPointIndex-1) - 1 + pointCount) % pointCount;
        int nextIndex = ((objectPointIndex-1) + 1) % pointCount;
        QPoint prePoint = polygon[preIndex];
        QPoint nextPoint = polygon[nextIndex];
        QPoint movePoint = polygon[objectPointIndex-1];
        QPoint point1 = movePoint - prePoint;
        QPoint point2 = movePoint - nextPoint;
        if(point1.manhattanLength() <= ManualParamterConfig::getNearPointLenght() ||
                point2.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
        {
            polygon.removeAt(objectPointIndex-1);
            objectList[nearObjectIndex].setPointList(polygon);
        }
    }
    drawMousePressed = false;
    moveMousePressed = false;
    isDraw = true;
    return 0;
}

int DrawPointsShape::drawMouseDoubleClick(QWidget *parent, const QPoint point, bool &isDraw)
{
    if(pointsList.count() >= 1)
    {
        MyObject object;
        object.setShapeType(ShapeType::POINT_SHAPE);
        object.setPointList(pointsList);
        objectList.append(object);
    }
    else
    {
        QMessageBox::information(parent, tr("标注"), tr("标注目标点数需要大于1!"));
    }
    pointsList.clear();
    firstPoint = QPoint(-1, -1);
    drawMousePressed = false;
    moveMousePressed = false;
    isDraw = true;
    return 0;
}

void DrawPointsShape::removeShape(bool &isDraw)
{
    isDraw = false;
    if(removeObjectIndex >= 0 && removeObjectIndex < objectList.count())
    {
        this->objectList.removeAt(removeObjectIndex);
        removeObjectIndex = -1;
        isDraw = true;
    }
}

bool DrawPointsShape::isInShape(const QPoint &point)
{
    bool isFind = false;
    removeObjectIndex = objectList.count();
    for(int loop = 0; loop < objectList.count(); loop++)
    {
        QList<QPoint> polygon = objectList[loop].getPointList();
        int index = 0;
        for(; index < polygon.count(); index++)
        {
            QPoint diffPoint = polygon[index] - point;
            if(diffPoint.manhattanLength() < 20)
            {
                removeObjectIndex = loop;
                isFind = true;
                break;
            }
        }
        if(index < polygon.count())
        {
            break;
        }
    }
    return isFind;
}

void DrawPointsShape::drawPixmap(const ShapeType shapeID, QPainter &painter)
{
    const int height = painter.device()->height();
    const int width = painter.device()->width();
    int drawLineWidth = 2;
    QPen pen(QColor("#3CFF55"), drawLineWidth ,Qt::DashLine);
    QFont font("Decorative", 10);
    painter.setPen(pen);
    painter.setFont(font);
    painter.setBrush(QColor("#3CFF55"));

    if(shapeID == ShapeType::LANE_SEGMENT_SHAPE)
    {
        painter.save();
        QPen tempPen(QColor(0, 0, 0, 120));
        tempPen.setWidth(drawLineWidth);
        painter.setPen(tempPen);
        for(int loop = 10; loop < height; loop += 10)
        {
            painter.drawLine(QPoint(0, loop), QPoint(width, loop));
        }
        painter.restore();
    }

    for(int i=0; i< this->objectList.count(); i++)
    {
        QString color = ManualParamterConfig::getMarkClassColor(this->objectList[i].getObjectClass());
        QColor drawColor(color);
        if(drawColor.isValid())
        {
            pen.setColor(drawColor);
            painter.setPen(pen);
        }
        else
        {
            drawColor = QColor("#000000");
            pen.setColor(drawColor);
            painter.setPen(pen);
        }
        if(this->visibleSampleClass == "All")
        {
            drawMark(this->objectList[i].getPointList(), painter);
            painter.drawText(this->objectList[i].getPointList().at(0), this->objectList[i].getObjectClass());
        }
        else
        {
            if(this->objectList[i].getObjectClass().contains(this->visibleSampleClass))
            {
                drawMark(this->objectList[i].getPointList(), painter);
                painter.drawText(this->objectList[i].getPointList().at(0), this->objectList[i].getObjectClass());
            }
        }
    }

    if(!this->pointsList.isEmpty())
    {
        painter.setBrush(QColor("#3CFF55"));
        foreach(QPoint var, this->pointsList)
        {
            painter.drawEllipse(var, 2, 2);
        }
        painter.setBrush(QColor("#000000"));
        painter.drawEllipse(firstPoint, 4, 4);
    }
}

void DrawPointsShape::setObjectList(QList<MyObject> list)
{
    this->objectList.clear();
    this->objectList = list;
}

void DrawPointsShape::getObjectList(QList<MyObject> &list)
{
    list = this->objectList;
}

int DrawPointsShape::getObjectSize()
{
    return this->objectList.count();
}

int DrawPointsShape::nearObjectPoint(const QPoint point)
{
    int resultIndex = -1;
    int moveCount = objectList.size();
    if(firstPoint.x() >= 0 && firstPoint.y() >= 0)
    {
        QPoint point1 = point - firstPoint;
        if(point1.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
        {
            nearFirstPoint = true;
        }
        else
        {
            nearFirstPoint = false;
        }
    }
    else
    {
        nearFirstPoint = false;
    }
    for(int index = 0; index < moveCount; index++)
    {
        QList<QPoint> polygon = objectList[index].getPointList();
        for(int pointIndex = 0; pointIndex < polygon.count(); pointIndex++)
        {
           QPoint point1 = point - polygon[pointIndex];
           if(point1.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
           {
               objectPointIndex = pointIndex + 1;
               resultIndex = index;
               return resultIndex;
           }
        }
    }
    return resultIndex;
}

void DrawPointsShape::updateObject(const QPoint point)
{
    QList<QPoint> polygon = objectList[nearObjectIndex].getPointList();
    polygon[objectPointIndex - 1] = point;
    objectList[nearObjectIndex].setPointList(polygon);
}

void DrawPointsShape::drawMark(const QList<QPoint> &pointList, QPainter &painter)
{
    for(int loop = 0; loop < pointList.size(); loop++)
    {
        painter.drawEllipse(pointList[loop], 2, 2);
    }
}
