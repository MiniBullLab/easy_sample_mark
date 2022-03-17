#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "imagetrackingcontrolwindow.h"
#include <QImageReader>
#include <QMessageBox>
#include <QDebug>

ImageTrackingControlWindow::ImageTrackingControlWindow(QWidget *parent)
    : ControlWindow(parent)
{
    initDrawWidget();
    initData();
    initConnect();
}

ImageTrackingControlWindow::~ImageTrackingControlWindow()
{

}

void ImageTrackingControlWindow::setMarkDataList(const QString markDataDir, const QList<QString> markDataList, const MarkDataType dataType)
{
    markDataListWidget->clear();
    initMarkData(markDataDir, dataType);
    initImageData();

    updateIsMarkButton(this->isMark);
    updateDrawLabel(this->isMark);

    if(markDataList.size() > 0)
    {
        readClassConfig();
        this->processMarkDataList = markDataList;
        initImageList();
        updateListBox();
        updateMarkProcessLable();
        isMarkButton->setEnabled(true);
    }
    else
    {
        isMarkButton->setEnabled(false);
    }
    this->setFocus();
}

void ImageTrackingControlWindow::saveMarkDataList()
{
    if(this->isMark)
    {
        saveMarkDataResult();
    }
    writeMarkHistory();
}

void ImageTrackingControlWindow::setDrawShape(int shapeId)
{
    this->drawLable->setDrawShape(shapeId);
}


void ImageTrackingControlWindow::slotImageItem(QListWidgetItem *item)
{
    if(this->isMark)
    {
        saveMarkDataResult();
    }
    this->currentIndex = markDataListWidget->row(item);
    loadMarkData(item->text());
    this->setFocus();
}

void ImageTrackingControlWindow::slotChangeClass(QString classText)
{
    if(this->isMark)
    {
        saveMarkDataResult();
    }
    if(this->currentIndex >= 0)
    {
        loadMarkImage();
        this->setFocus();
    }
}

void ImageTrackingControlWindow::slotScrollArea(int keyValue)
{
    if(processMarkDataList.size() > 0)
    {
        switch (keyValue)
        {
        case Qt::Key_A:
            showPrevious();
            break;
        case Qt::Key_D:
            showNext();
            break;
        case Qt::Key_E:
            slotIsMark();
            break;
        }
        if(keyValue == int(Qt::ControlModifier + Qt::Key_Z))
        {
            if(isMark)
            {
                drawLable->undoDrawShape();
            }
        }
    }
    if(keyValue == int(Qt::Key_Escape))
    {
        slotShowFull();
    }
}

void ImageTrackingControlWindow::closeEvent(QCloseEvent *event)
{
    saveMarkDataList();
    ControlWindow::closeEvent(event);
}

void ImageTrackingControlWindow::keyPressEvent(QKeyEvent *event)
{
    if(processMarkDataList.size() > 0)
    {
        switch (event->key())
        {
        case Qt::Key_A:
            showPrevious();
            break;
        case Qt::Key_D:
            showNext();
            break;
        case Qt::Key_E:
            slotIsMark();
            break;
        }
    }
    if(event->modifiers() == Qt::ControlModifier)
    {
        if(event->key() == Qt::Key_M)
        {
            this->setWindowState(Qt::WindowMaximized);
        }
        else if(event->key() == Qt::Key_Z && isMark)
        {
            drawLable->undoDrawShape();
        }
    }
}

void ImageTrackingControlWindow::isMarkData()
{
    if(currentIndex >= 0)
    {
        if(isMark)
        {
            saveMarkDataResult();
            this->isMark = false;
        }
        else
        {
            this->isMark = true;
        }
        updateIsMarkButton(this->isMark);
        updateDrawLabel(this->isMark);
    }
}

void ImageTrackingControlWindow::resetDraw()
{
    drawLable->resetDraw();
}

void ImageTrackingControlWindow::showPrevious()
{
    if(this->markDataType == MarkDataType::IMAGE_TRACKING)
    {
        if(currentIndex > 0)
        {
            if(this->isMark)
                saveMarkDataResult();
            currentIndex--;
            loadMarkImage();
        }
    }
}

void ImageTrackingControlWindow::showNext()
{
    if(currentIndex < processMarkDataList.size() - 1)
    {
        if(this->isMark)
            saveMarkDataResult();
        currentIndex++;
        loadMarkImage();
    }
}

void ImageTrackingControlWindow::updateDrawLabel(bool isValue)
{
    this->drawLable->setEnabled(isValue);
}

void ImageTrackingControlWindow::updateImage()
{
    drawLable->clearDraw();
    drawLable->setNewQImage(currentImage);
}

void ImageTrackingControlWindow::loadMarkData(const QString dataPath)
{   QString saveAnnotationsDir = this->markDataDir + "/../" + "Annotations";
    if(this->markDataType == MarkDataType::IMAGE_TRACKING)
    {
        loadImageData(dataPath, saveAnnotationsDir);
    }
    updateListBox();
    updateMarkProcessLable();
}

void ImageTrackingControlWindow::saveMarkDataResult()
{
    QDir makeDir;
    QString saveAnnotationsDir = this->markDataDir + "/../" + "Annotations";
    QList<MyObject> objects = drawLable->getObjects();
    if(objects.size() > 0)
    {
        if(!makeDir.exists(saveAnnotationsDir))
        {
            if(!makeDir.mkdir(saveAnnotationsDir))
            {
                qDebug() << "make Annotations dir fail!" << endl;
            }
        }
        if(this->markDataType == MarkDataType::IMAGE_TRACKING)
        {
            saveImageDataResult(saveAnnotationsDir, this->currentImagePath, objects);
        }
    }
}

void ImageTrackingControlWindow::loadMarkImage()
{
    QString saveAnnotationsDir = this->markDataDir + "/../" + "Annotations";
    if(this->markDataType == MarkDataType::IMAGE_TRACKING && processMarkDataList.size() > 0)
    {
        currentImagePath =  processMarkDataList[currentIndex];
        loadImageData(currentImagePath, saveAnnotationsDir);
    }
    updateListBox();
    updateMarkProcessLable();
}

void ImageTrackingControlWindow::initDrawWidget()
{
    drawLable = new Tracking2DLabel();
    drawLableScrollArea = new QScrollArea();
    drawLableScrollArea->setAlignment(Qt::AlignCenter);
    drawLableScrollArea->setBackgroundRole(QPalette::Dark);
    drawLableScrollArea->setAutoFillBackground(true);
    //drawScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  //控件大小 小于 视窗大小时，默认不会显示滚动条
    //drawScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    //强制显示滚动条。
    drawLableScrollArea->setWidget(drawLable);
    drawMarkDataWidget->addWidget(drawLableScrollArea);

    drawLable->clearDraw();
    drawLable->setNewQImage(currentImage);
    drawLable->setEnabled(false);
    drawMarkDataWidget->setCurrentIndex(1);
}

void ImageTrackingControlWindow::initData()
{
    initMarkData(".", MarkDataType::IMAGE_TRACKING);
    initImageData();
}

void ImageTrackingControlWindow::initConnect()
{
    connect(markDataListWidget, &QListWidget::itemClicked, this, &ImageTrackingControlWindow::slotImageItem);
    connect(drawMarkDataWidget, &MyStackedWidget::signalsKey, this, &ImageTrackingControlWindow::slotScrollArea);
    connect(classBox, &QComboBox::currentTextChanged, this, &ImageTrackingControlWindow::slotChangeClass);
}

void ImageTrackingControlWindow::initImageList()
{
    processDataFlagList.clear();
    readMarkHistory();
}

void ImageTrackingControlWindow::loadImageData(const QString imagePath, const QString saveAnnotationsDir)
{
    if(currentImage.load(imagePath))
    {
        currentImagePath = imagePath;
        updateImage();
        QFileInfo imageFileInfo(currentImagePath);
        QString readJsonPath= saveAnnotationsDir + "/" + imageFileInfo.completeBaseName() + ".json";
        QFileInfo jsonFileInfo(readJsonPath);
        QList<MyObject> objects;
        if(jsonFileInfo.exists() && jsonProcess.readJSON(readJsonPath, objects) == 0)
        {
            drawLable->setOjects(objects, classBox->currentText());
        }
//        currentImagePath = imagePath;
//        updateImage();
//        QFileInfo imageFileInfo(currentImagePath);
//        QString readXmlPath= saveAnnotationsDir + "/" + imageFileInfo.completeBaseName() + ".xml";
//        QFileInfo xmlFileInfo(readXmlPath);
//        QList<MyObject> objects;
//        if(xmlFileInfo.exists() && xmlProcess.readXML(readXmlPath, objects) == 0)
//        {
//            drawLable->setOjects(objects, classBox->currentText());
//        }
    }
    else
    {
        QMessageBox::information(this, tr("加载图片"), tr("该图片加载失败,请点击下一张图片！"));
    }
}

void ImageTrackingControlWindow::saveImageDataResult(const QString &saveAnnotationsDir, const QString &imagePath,
                                                    const QList<MyObject> &objects)
{
    QFileInfo imageFileInfo(imagePath);
    QString saveJsonPath = saveAnnotationsDir + "/" + imageFileInfo.completeBaseName() + ".json";
    QFileInfo jsonFileInfo(saveJsonPath);

    QMessageBox::StandardButton result = QMessageBox::question(this, tr("保存标注信息"), tr("是否保存标注信息?"),
                                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(result == QMessageBox::Yes)
    {
        if(objects.size() > 0)
        {
            if(jsonProcess.createJSON(saveJsonPath, currentImagePath, currentImage.width(),
                                      currentImage.height(), objects) == 0)
            {
                if(currentIndex >= 0)
                {
                    processDataFlagList[currentIndex] = 0;
                }
            }
            else
            {
                QMessageBox::information(this, tr("保存Json"), tr("保存Json文件失败！"));
            }
        }
        else if(jsonFileInfo.exists())
        {
            QFile tempFile(saveJsonPath);
            tempFile.remove();
        }
    }
//    QFileInfo imageFileInfo(imagePath);
//    QString saveXmlPath = saveAnnotationsDir + "/" + imageFileInfo.completeBaseName() + ".xml";
//    QFileInfo xmlFileInfo(saveXmlPath);

//    QMessageBox::StandardButton result = QMessageBox::question(this, tr("保存标注信息"), tr("是否保存标注信息?"),
//                                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
//    if(result == QMessageBox::Yes)
//    {
//        if(objects.size() > 0)
//        {
//            if(xmlProcess.createXML(saveXmlPath, currentImagePath, currentImage.width(),
//                                    currentImage.height(), objects) == 0)
//            {
//                if(currentIndex >= 0)
//                {
//                    processDataFlagList[currentIndex] = 0;
//                }
//            }
//            else
//            {
//                QMessageBox::information(this, tr("保存XML"), tr("保存XML文件失败！"));
//            }
//        }
//        else if(xmlFileInfo.exists())
//        {
//            QFile tempFile(saveXmlPath);
//            tempFile.remove();
//        }
//    }
}

void ImageTrackingControlWindow::initImageData()
{
    this->currentImagePath = "";
}

