#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "drawrecttrackshape.h"
#include <QMessageBox>
#include "sampleMarkParam/manualparamterconfig.h"
#include "selectMarkInfo/selectmarktrackingwindow.h"

const static std::vector<QString> colorDefine = {"#FFFFFF", "#FFFF00",  "#0000FF",
                                                "#00FF00", "#00FFFF", "#FF0000",
                                                "#FF00FF", "#000000",
                                                "#808080", "#808000", "#000080",
                                                "#008000", "#008080", "#800000",
                                                "#CC99FF", "#CC3399", "#660033",
                                                "#660000", "#3399FF", "#EEDC82",
                                                "#A0522D", "#B8860B", "#7A378B",
                                                "#5F9EA0", "#828282", "#00CDCD"};

DrawRectTrackShape::DrawRectTrackShape(MarkDataType dataType, QObject *parent) : DrawShape(dataType, parent)
{
    initDraw();
}

DrawRectTrackShape::~DrawRectTrackShape()
{

}

void DrawRectTrackShape::initDraw()
{
    drawMousePressed = false;
    moveMousePressed = false;

    nearRectIndex = -1;
    rectPointIndex = 0;
    removeRectIndex = -1;
    listRect.clear();
}

int DrawRectTrackShape::drawMousePress(const QPoint point, bool &isDraw)
{
    int mouseChange = 0;
    nearRectIndex = nearRectPiont(point);
    if(nearRectIndex >= 0)
    {
        mouseChange = 2;
        drawMousePressed = false;
        moveMousePressed = true;
        updateRect(point);
    }
    else
    {
        mouseChange = 1;
        drawMousePressed = true;
        moveMousePressed = false;
        currentRect.setTopLeft(point);
        currentRect.setBottomRight(point);
    }
    isDraw = true;
    return mouseChange;
}

int DrawRectTrackShape::drawMouseMove(const QPoint point, bool &isDraw)
{
    int mouseChange = 0;
    isDraw = false;
    if(drawMousePressed)
    {
        currentRect.setBottomRight(point);
        mouseChange = 1;
        isDraw = true;
    }
    else
    {
        if(moveMousePressed)
        {
            updateRect(point);
            mouseChange = 2;
            isDraw = true;
        }
        else
        {
            nearRectIndex = nearRectPiont(point);
            if(nearRectIndex >= 0)
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

int DrawRectTrackShape::drawMouseRelease(QWidget *parent, const QPoint point, bool &isDraw)
{
    if(drawMousePressed)
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
            SelectMarkTrackingWindow *window = new SelectMarkTrackingWindow(this->markDataType);
            window->setModal(true);
            window->setObjectRect(this->visibleSampleClass);
            int res = window->exec();
            if (res == QDialog::Accepted)
            {
                MyObject object;
                object.setShapeType(ShapeType::RECT_SHAPE);
                object.setBox(currentRect);
                object.setID(window->getObjectId());
                object.setObjectClass(window->getObjectClass());
                object.setIsDifficult(window->getIsDifficult());
                object.setObjectFlag(window->getObjectFlag());
                listRect.append(object);
            }
            window->deleteLater();
        }
        else
        {
            QMessageBox::information(parent, tr("标注"), tr("标注目标宽度<%1或者高度<%2").arg(
                                         ManualParamterConfig::getMinWidth()).arg(ManualParamterConfig::getMinHeight()));
        }
    }
    else if(moveMousePressed)
    {
        QRect rect = listRect[nearRectIndex].getBox();
        int minWidth = ManualParamterConfig::getMinWidth();
        int minHeight = ManualParamterConfig::getMinHeight();
        if(rect.width() < minWidth && rect.height() < minHeight)
        {
            this->listRect.removeAt(nearRectIndex);
            QMessageBox::information(parent, tr("标注"), tr("标注目标宽度<%1或者高度<%2").arg(minWidth).arg(minHeight));
        }
    }
    drawMousePressed = false;
    moveMousePressed = false;
    isDraw = true;
    return 0;
}

void DrawRectTrackShape::removeShape(bool &isDraw)
{
    isDraw = false;
    if(removeRectIndex >= 0 && removeRectIndex < listRect.size())
    {
        this->listRect.removeAt(removeRectIndex);
        removeRectIndex = -1;
        isDraw = true;
    }
}

bool DrawRectTrackShape::isInShape(const QPoint &point)
{
    bool isFind = false;
    removeRectIndex = listRect.size();
    for(int loop = 0; loop < listRect.size(); loop++)
    {
        QRect rect = listRect[loop].getBox();
        if(rect.contains(point))
        {
            removeRectIndex = loop;
            isFind = true;
            break;
        }
    }
    return isFind;
}

void DrawRectTrackShape::drawPixmap(const ShapeType shapeID, QPainter &painter)
{
    QPen pen(QColor("#3CFF55"), 2 ,Qt::DashLine);
    QFont font("Decorative", 15);
    painter.setPen(pen);
    painter.setFont(font);

    bool isDraw = false;
    QRect currentRect;
    QList<QPoint> points = getRectListPoints(this->visibleSampleClass);
    getCurrentRect(currentRect, isDraw);

    for(int i = 0; i < this->listRect.size(); i++)
    {
        int index = this->listRect[i].getID() % colorDefine.size();
        QString color = colorDefine[index];
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
            painter.drawRect(this->listRect[i].getBox());
            painter.drawText(this->listRect[i].getBox().topLeft(), QString("%1 %2").
                             arg(this->listRect[i].getID()).
                                 arg(this->listRect[i].getObjectClass()));
        }
        else
        {
            if(this->listRect[i].getObjectClass().contains(this->visibleSampleClass))
            {
                painter.drawRect(this->listRect[i].getBox());
                painter.drawText(this->listRect[i].getBox().topLeft(), QString("%1 %2").
                                 arg(this->listRect[i].getID()).
                                     arg(this->listRect[i].getObjectClass()));
            }
        }
    }

    if(isDraw)
    {
        painter.drawRect(currentRect);
    }

    pen.setColor(QColor("#3CFF55"));
    painter.setPen(pen);
    painter.setBrush(QColor("#3CFF55"));
    for(int i = 0; i < points.size(); i++)
    {
        painter.drawEllipse(points[i], 2, 2);
    }
}

void DrawRectTrackShape::setObjectList(QList<MyObject> list)
{
    this->listRect.clear();
    this->listRect = list;
}

void DrawRectTrackShape::getObjectList(QList<MyObject> &list)
{
    list = this->listRect;
}

void DrawRectTrackShape::getCurrentRect(QRect &rect, bool &isDraw)
{
    if(drawMousePressed)
    {
        isDraw = true;
    }
    else
    {
        isDraw = false;
    }
    rect = this->currentRect;
}

QList<QPoint> DrawRectTrackShape::getRectListPoints(const QString sampleClass)
{
    QList<QPoint> pointList;
    pointList.clear();
    for(int index = 0; index < listRect.size(); index++)
    {
        if(sampleClass == "All")
        {
            QRect box = listRect[index].getBox();
            pointList.append(box.topLeft());
            pointList.append(box.topRight());
            pointList.append(box.bottomLeft());
            pointList.append(box.bottomRight());
        }
        else if(listRect[index].getObjectClass().contains(sampleClass))
        {
            QRect box = listRect[index].getBox();
            pointList.append(box.topLeft());
            pointList.append(box.topRight());
            pointList.append(box.bottomLeft());
            pointList.append(box.bottomRight());
        }
    }
    if(drawMousePressed)
    {
        pointList.append(currentRect.topLeft());
        pointList.append(currentRect.topRight());
        pointList.append(currentRect.bottomLeft());
        pointList.append(currentRect.bottomRight());
    }
    return pointList;
}

int DrawRectTrackShape::nearRectPiont(const QPoint point)
{
    int resultIndex = -1;
    for(int index = 0; index < listRect.size(); index++)
    {
        QRect box = listRect[index].getBox();
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

void DrawRectTrackShape::updateRect(const QPoint point)
{
    if(rectPointIndex  == 1)
    {
        QRect rect = listRect[nearRectIndex].getBox();
        rect.setTopLeft(point);
        listRect[nearRectIndex].setBox(rect);
        listRect[nearRectIndex].setIsTrackingObject(false);
    }
    else if(rectPointIndex  == 2)
    {
        QRect rect = listRect[nearRectIndex].getBox();
        rect.setTopRight(point);
        listRect[nearRectIndex].setBox(rect);
        listRect[nearRectIndex].setIsTrackingObject(false);
    }
    else if(rectPointIndex  == 3)
    {
        QRect rect = listRect[nearRectIndex].getBox();
        rect.setBottomLeft(point);
        listRect[nearRectIndex].setBox(rect);
        listRect[nearRectIndex].setIsTrackingObject(false);
    }
    else if(rectPointIndex  == 4)
    {
        QRect rect = listRect[nearRectIndex].getBox();
        rect.setBottomRight(point);
        listRect[nearRectIndex].setBox(rect);
        listRect[nearRectIndex].setIsTrackingObject(false);
    }
}
