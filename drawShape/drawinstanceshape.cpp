#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "drawinstanceshape.h"
#include <QMessageBox>
#include "sampleMarkParam/manualparamterconfig.h"
#include "selectmarkclasswindow.h"

DrawInstanceShape::DrawInstanceShape(MarkDataType dataType, bool isSegment, QObject *parent) :
    DrawShape(dataType, parent), isSegment(isSegment)
{
    initDraw();
}

DrawInstanceShape::~DrawInstanceShape()
{

}

void DrawInstanceShape::initDraw()
{
    drawMousePressed = false;
    moveMousePressed = false;

    finishDrawRect = false;
    finishDrawPolygon = false;

    nearFirstPoint = false;
    firstPoint = QPoint(-1, -1);
    currentPolygon.clear();

    nearPolygonIndex = -1;
    polygonPointIndex = 0;

    nearRectIndex = -1;
    rectPointIndex = 0;

    removeRectIndex = -1;

    currentRect = QRect(0, 0, -1, -1);

    listInstance.clear();
}

int DrawInstanceShape::drawMousePress(const QPoint point, bool &isDraw)
{
    int mouseChange = 0;
    if(nearRectIndex >= 0)
    {
        mouseChange = 2;
        drawMousePressed = false;
        moveMousePressed = true;
        updateRect(point);
    }
    else if(nearPolygonIndex >= 0)
    {
        mouseChange = 2;
        drawMousePressed = false;
        moveMousePressed = true;
        updatePolygon(point);
    }
    else
    {
        if(!finishDrawRect)
        {
            mouseChange = 1;
            currentRect.setTopLeft(point);
            currentRect.setBottomRight(point);
        }
        else
        {
            if(firstPoint.x() < 0 || firstPoint.y() < 0)
            {
                firstPoint = point;
            }
            if(!nearFirstPoint)
            {
                currentPolygon.append(point);
            }
            else
            {
                finishDrawPolygon = true;
            }
            mouseChange = 1;
        }
        drawMousePressed = true;
        moveMousePressed = false;
    }
    isDraw = true;
    return mouseChange;
}

int DrawInstanceShape::drawMouseMove(const QPoint point, bool &isDraw)
{
    int mouseChange = 0;
    isDraw = false;
    if(drawMousePressed && !finishDrawRect)
    {
        currentRect.setBottomRight(point);
        mouseChange = 1;
        isDraw = true;
    }
    else
    {
        if(nearRectIndex >= 0 && moveMousePressed)
        {
            updateRect(point);
            mouseChange = 2;
            isDraw = true;
        }
        else if(nearPolygonIndex >= 0 && moveMousePressed)
        {
            updatePolygon(point);
            mouseChange = 2;
            isDraw = true;
        }
        else
        {
            nearRectIndex = nearRectPiont(point);
            nearPolygonIndex = nearPolygonPoint(point);
            if(nearRectIndex >= 0 || nearPolygonIndex >= 0 || nearFirstPoint)
            {
                mouseChange = 2;
            }
            else
            {
                mouseChange = 1;
            }
        }
    }
    return mouseChange;
}

int DrawInstanceShape::drawMouseRelease(QWidget *parent, const QPoint point, bool &isDraw)
{
    if(drawMousePressed && !finishDrawRect)
    {
        int rectMinX = std::min(currentRect.topLeft().x(), currentRect.bottomRight().x());
        int rectMinY = std::min(currentRect.topLeft().y(), currentRect.bottomRight().y());
        int rectMaxX = std::max(currentRect.topLeft().x(), currentRect.bottomRight().x());
        int rectMaxY = std::max(currentRect.topLeft().y(), currentRect.bottomRight().y());
        currentRect.setTopLeft(QPoint(rectMinX, rectMinY));
        currentRect.setBottomRight(QPoint(rectMaxX, rectMaxY));
        if(currentRect.width() >= ManualParamterConfig::getMinWidth()
                && currentRect.height() >= ManualParamterConfig::getMinHeight())
        {
            finishDrawRect = true;
        }
        else
        {
            finishDrawRect = false;
            currentRect = QRect(0, 0, -1, -1);
            QMessageBox::information(parent, tr("标注"), tr("标注目标宽度<%1或者高度<%2,请重新标矩形框!").arg(
                                         ManualParamterConfig::getMinWidth()).arg(ManualParamterConfig::getMinHeight()));
        }
    }
    else if(finishDrawRect && finishDrawPolygon)
    {
        if(currentPolygon.count() > 2)
        {
            float area = geometryAlgorithm.polygonArea(currentPolygon);
            int minArea = ManualParamterConfig::getMinWidth() * ManualParamterConfig::getMinHeight();
            if(area >= minArea)
            {
                SelectMarkClassWindow *window = new SelectMarkClassWindow(this->markDataType);
                window->setModal(true);
                window->setObjectRect(this->visibleSampleClass);
                int res = window->exec();
                if (res == QDialog::Accepted)
                {
                    MyObject object;
                    if(this->markDataType == MarkDataType::SEGMENT)
                    {
                        object.setShapeType(ShapeType::INSTANCE_SEGMENT_SHAPE);
                    }
                    object.setBox(currentRect);
                    object.setMask(currentPolygon);
                    object.setObjectClass(window->getObjectClass());
                    object.setIsDifficult(window->getIsDifficult());
                    object.setObjectFlag(window->getObjectFlag());
                    listInstance.append(object);
                }
                window->deleteLater();
                currentRect = QRect(0, 0, -1, -1);
                currentPolygon.clear();
                firstPoint = QPoint(-1, -1);
                finishDrawRect = false;
                finishDrawPolygon = false;
            }
            else
            {
                QMessageBox::information(parent, tr("标注"), tr("标注目标多边形面积小于%1, 请重新标多边形!").arg(minArea));
                currentPolygon.clear();
                firstPoint = QPoint(-1, -1);
                finishDrawPolygon = false;
            }
        }
        else
        {
            QMessageBox::information(parent, tr("标注"), tr("标注目标有误, 请重新标多边形!"));
            currentPolygon.clear();
            firstPoint = QPoint(-1, -1);
            finishDrawPolygon = false;
        }
    }
    else if(nearRectIndex >= 0 && moveMousePressed)
    {
        QRect rect = listInstance[nearRectIndex].getBox();
        int minWidth = ManualParamterConfig::getMinWidth();
        int minHeight = ManualParamterConfig::getMinHeight();
        if(rect.width() < minWidth || rect.height() < minHeight)
        {
            this->listInstance.removeAt(nearRectIndex);
            currentRect = QRect(0, 0, -1, -1);
            currentPolygon.clear();
            firstPoint = QPoint(-1, -1);
            finishDrawRect = false;
            finishDrawPolygon = false;
            QMessageBox::information(parent, tr("标注"), tr("标注目标宽度<%1或者高度<%2").arg(minWidth).arg(minHeight));
        }
    }
    else if(nearPolygonIndex >= 0 && moveMousePressed)
    {
        QPolygon polygon = listInstance[nearPolygonIndex].getMask();
        int pointCount = polygon.count();
        int preIndex = ((polygonPointIndex-1) - 1 + pointCount) % pointCount;
        int nextIndex = ((polygonPointIndex-1) + 1) % pointCount;
        QPoint prePoint = polygon[preIndex];
        QPoint nextPoint = polygon[nextIndex];
        QPoint movePoint = polygon[polygonPointIndex-1];
        QPoint point1 = movePoint - prePoint;
        QPoint point2 = movePoint - nextPoint;
        if(point1.manhattanLength() <= ManualParamterConfig::getNearPointLenght() ||
                point2.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
        {
            polygon.removeAt(polygonPointIndex-1);
            if(polygon.count() <= 2)
            {
                this->listInstance.removeAt(nearPolygonIndex);
                currentRect = QRect(0, 0, -1, -1);
                currentPolygon.clear();
                firstPoint = QPoint(-1, -1);
                finishDrawRect = false;
                finishDrawPolygon = false;
                QMessageBox::information(parent, tr("标注"), tr("标注目标有误!"));
            }
            else
            {
                listInstance[nearPolygonIndex].setPolygon(polygon);
            }
        }
    }
    drawMousePressed = false;
    moveMousePressed = false;
    isDraw = true;
    return 0;
}

void DrawInstanceShape::removeShape(bool &isDraw)
{
    isDraw = false;
    if(removeRectIndex >= 0 && removeRectIndex < listInstance.size())
    {
        this->listInstance.removeAt(removeRectIndex);
        removeRectIndex = -1;
        isDraw = true;
    }
}

bool DrawInstanceShape::isInShape(const QPoint &point)
{
    bool isFind = false;
    removeRectIndex = listInstance.size();
    for(int loop = 0; loop < listInstance.size(); loop++)
    {
        QRect rect = listInstance[loop].getBox();
        if(rect.contains(point))
        {
            removeRectIndex = loop;
            isFind = true;
            break;
        }
    }
    return isFind;
}

void DrawInstanceShape::cancelDrawShape(bool &isDraw)
{
    isDraw = false;
    if(this->currentPolygon.count() > 0)
    {
        this->currentPolygon.pop_back();
        isDraw = true;
    }
}

void DrawInstanceShape::drawPixmap(const ShapeType shapeID, QPainter &painter)
{
    int drawLineWidth = 2;
    if(isSegment)
    {
        drawLineWidth = 1;
    }
    QPen pen(QColor("#3CFF55"), drawLineWidth ,Qt::DashLine);
    QFont font("Decorative", 15);
    painter.setPen(pen);
    painter.setFont(font);

    QList<QPoint> points = getRectListPoints(this->visibleSampleClass);

    for(int i = 0; i < this->listInstance.size(); i++)
    {
        QString color = ManualParamterConfig::getMarkClassColor(this->listInstance[i].getObjectClass());
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
            painter.drawRect(this->listInstance[i].getBox());
            QPolygon drawpoints = this->listInstance[i].getMask();
            drawpoints.append(drawpoints.at(0));
            QPainterPath path;
            path.addPolygon(this->listInstance[i].getMask());
            painter.fillPath(path, QBrush(QColor(drawColor.red(), drawColor.green(),
                                                drawColor.blue(), 80)));
            foreach (QPoint var, this->listInstance[i].getMask())
            {
                painter.drawEllipse(var, 2, 2);
            }
            painter.drawPolyline(QPolygon(drawpoints));

            painter.drawText(this->listInstance[i].getBox().topLeft(), this->listInstance[i].getObjectClass());
        }
        else
        {
            if(this->listInstance[i].getObjectClass().contains(this->visibleSampleClass))
            {
                painter.drawRect(this->listInstance[i].getBox());
                QPolygon drawpoints = this->listInstance[i].getMask();
                drawpoints.append(drawpoints.at(0));
                QPainterPath path;
                path.addPolygon(this->listInstance[i].getMask());
                painter.fillPath(path, QBrush(QColor(drawColor.red(), drawColor.green(),
                                                    drawColor.blue(), 80)));
                foreach (QPoint var, this->listInstance[i].getMask())
                {
                    painter.drawEllipse(var, 2, 2);
                }
                painter.drawPolyline(QPolygon(drawpoints));
                painter.drawText(this->listInstance[i].getBox().topLeft(), this->listInstance[i].getObjectClass());
            }
        }
    }

    if(currentRect.width() * currentRect.height() > 0)
    {
        painter.drawRect(currentRect);
    }
    if(!currentPolygon.isEmpty())
    {
        painter.setBrush(QColor("#3CFF55"));
        foreach (QPoint var, currentPolygon)
        {
            painter.drawEllipse(var, 2, 2);
        }
        painter.setBrush(QColor("#000000"));
        painter.drawEllipse(firstPoint, 4, 4);
        QPen pen(QColor("#3CFF55"), drawLineWidth ,Qt::DashLine);
        painter.setPen(pen);
        painter.drawPolyline(QPolygonF(currentPolygon));
    }

    pen.setColor(QColor("#3CFF55"));
    painter.setPen(pen);
    painter.setBrush(QColor("#3CFF55"));
    for(int i = 0; i < points.size(); i++)
    {
        painter.drawEllipse(points[i], 2, 2);
    }
}

void DrawInstanceShape::setObjectList(QList<MyObject> list)
{
    this->listInstance.clear();
    this->listInstance = list;
}

void DrawInstanceShape::getObjectList(QList<MyObject> &list)
{
    list = this->listInstance;
}

QList<QPoint> DrawInstanceShape::getRectListPoints(const QString sampleClass)
{
    QList<QPoint> pointList;
    pointList.clear();
    for(int index = 0; index < listInstance.size(); index++)
    {
        if(sampleClass == "All")
        {
            QRect box = listInstance[index].getBox();
            pointList.append(box.topLeft());
            pointList.append(box.topRight());
            pointList.append(box.bottomLeft());
            pointList.append(box.bottomRight());
        }
        else if(listInstance[index].getObjectClass().contains(sampleClass))
        {
            QRect box = listInstance[index].getBox();
            pointList.append(box.topLeft());
            pointList.append(box.topRight());
            pointList.append(box.bottomLeft());
            pointList.append(box.bottomRight());
        }
    }
    if(currentRect.width() * currentRect.height() > 0)
    {
        pointList.append(currentRect.topLeft());
        pointList.append(currentRect.topRight());
        pointList.append(currentRect.bottomLeft());
        pointList.append(currentRect.bottomRight());
    }
    return pointList;
}

int DrawInstanceShape::nearRectPiont(const QPoint point)
{
    int resultIndex = -1;
    for(int index = 0; index < listInstance.size(); index++)
    {
        QRect box = listInstance[index].getBox();
        QPoint point1 = point - box.topLeft();
        if(point1.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
        {
            rectPointIndex = 1;
            resultIndex = index;
            break;
        }
        QPoint point2 = point - box.topRight();
        if(point2.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
        {
            rectPointIndex = 2;
            resultIndex = index;
            break;
        }
        QPoint point3 = point - box.bottomLeft();
        if(point3.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
        {
            rectPointIndex = 3;
            resultIndex = index;
            break;
        }
        QPoint point4 = point - box.bottomRight();
        if(point4.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
        {
            rectPointIndex = 4;
            resultIndex = index;
            break;
        }
    }
    return resultIndex;
}

void DrawInstanceShape::updateRect(const QPoint point)
{
    if(rectPointIndex  == 1)
    {
        QRect rect = listInstance[nearRectIndex].getBox();
        rect.setTopLeft(point);
        listInstance[nearRectIndex].setBox(rect);
        listInstance[nearRectIndex].setIsTrackingObject(false);
    }
    else if(rectPointIndex  == 2)
    {
        QRect rect = listInstance[nearRectIndex].getBox();
        rect.setTopRight(point);
        listInstance[nearRectIndex].setBox(rect);
        listInstance[nearRectIndex].setIsTrackingObject(false);
    }
    else if(rectPointIndex  == 3)
    {
        QRect rect = listInstance[nearRectIndex].getBox();
        rect.setBottomLeft(point);
        listInstance[nearRectIndex].setBox(rect);
        listInstance[nearRectIndex].setIsTrackingObject(false);
    }
    else if(rectPointIndex  == 4)
    {
        QRect rect = listInstance[nearRectIndex].getBox();
        rect.setBottomRight(point);
        listInstance[nearRectIndex].setBox(rect);
        listInstance[nearRectIndex].setIsTrackingObject(false);
    }
}

int DrawInstanceShape::nearPolygonPoint(const QPoint point)
{
    int resultIndex = -1;
    int moveCount = listInstance.size();
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
        QPolygon polygon = listInstance[index].getMask();
        for(int pointIndex = 0; pointIndex < polygon.count(); pointIndex++)
        {
           QPoint point1 = point - polygon[pointIndex];
           if(point1.manhattanLength() <= ManualParamterConfig::getNearPointLenght())
           {
               polygonPointIndex = pointIndex + 1;
               resultIndex = index;
               return resultIndex;
           }
        }
    }
    return resultIndex;
}

void DrawInstanceShape::updatePolygon(const QPoint point)
{
    QPolygon polygon = listInstance[nearPolygonIndex].getMask();
    polygon[polygonPointIndex - 1] = point;
    listInstance[nearPolygonIndex].setMask(polygon);
}


