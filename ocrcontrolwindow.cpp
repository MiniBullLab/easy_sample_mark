 #ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "ocrcontrolwindow.h"
#include <QImageReader>
#include <QMessageBox>
#include <QDebug>

OCRControlWindow::OCRControlWindow(QWidget *parent)
    : ControlWindow(parent)
{
    initDrawWidget();
    initData();
    initConnect();
}

OCRControlWindow::~OCRControlWindow()
{

}

void OCRControlWindow::setMarkDataList(const QString markDataDir, const QList<QString> markDataList, const MarkDataType dataType)
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

void OCRControlWindow::saveMarkDataList()
{
    if(this->isMark)
    {
        saveMarkDataResult();
    }
    writeMarkHistory();
}

void OCRControlWindow::setDrawShape(int shapeId)
{
    this->drawLable->setDrawShape(shapeId);
}

void OCRControlWindow::slotImageItem(QListWidgetItem *item)
{
    if(this->isMark)
    {
        saveMarkDataResult();
    }
    this->currentIndex = markDataListWidget->row(item);
    loadMarkData(item->text());
    this->setFocus();
}

void OCRControlWindow::slotChangeClass(QString classText)
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

void OCRControlWindow::slotScrollArea(int keyValue)
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

void OCRControlWindow::closeEvent(QCloseEvent *event)
{
    saveMarkDataList();
    ControlWindow::closeEvent(event);
}

void OCRControlWindow::keyPressEvent(QKeyEvent *event)
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

void OCRControlWindow::isMarkData()
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

void OCRControlWindow::resetDraw()
{
    drawLable->resetDraw();
}

void OCRControlWindow::showPrevious()
{
    if(this->markDataType == MarkDataType::SEGMENT)
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

void OCRControlWindow::showNext()
{
    if(currentIndex < processMarkDataList.size() - 1)
    {
        if(this->isMark)
            saveMarkDataResult();
        currentIndex++;
        loadMarkImage();
    }
}

void OCRControlWindow::updateDrawLabel(bool isValue)
{
    this->drawLable->setEnabled(isValue);
}

void OCRControlWindow::updateImage()
{
    drawLable->clearDraw();
    drawLable->setNewQImage(currentImage);
}

void OCRControlWindow::loadMarkData(const QString dataPath)
{
    QString saveAnnotationsDir = this->markDataDir + "/../" + "Annotations";
    if(this->markDataType == MarkDataType::OCR)
    {
        loadImageData(dataPath, saveAnnotationsDir);
    }
    updateListBox();
    updateMarkProcessLable();
}

void OCRControlWindow::saveMarkDataResult()
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
        if(this->markDataType == MarkDataType::OCR)
        {
            saveImageDataResult(saveAnnotationsDir, this->currentImagePath, objects);
        }
    }
    else
    {
        if(this->markDataType == MarkDataType::OCR)
        {
            saveImageDataResult(saveAnnotationsDir, this->currentImagePath, objects);
        }
    }
}

void OCRControlWindow::loadMarkImage()
{
    QString saveAnnotationsDir = this->markDataDir + "/../" + "Annotations";
    if(this->markDataType == MarkDataType::OCR && processMarkDataList.size() > 0)
    {
        currentImagePath =  processMarkDataList[currentIndex];
        loadImageData(currentImagePath, saveAnnotationsDir);
    }
    updateListBox();
    updateMarkProcessLable();
}

void OCRControlWindow::initDrawWidget()
{
    drawLable = new OCRLabel();
    drawLableScrollArea = new QScrollArea();
    drawLableScrollArea->setAlignment(Qt::AlignCenter);
    drawLableScrollArea->setBackgroundRole(QPalette::Dark);
    drawLableScrollArea->setAutoFillBackground(true);
    //drawScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  //控件大小 小于 视窗大小时，默认不会显示滚动条
    //drawScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    //强制显示滚动条。
    drawLableScrollArea->setWidget(drawLable);
    drawMarkDataWidget->addWidget(drawLableScrollArea);

    classBox->clear();
    classBox->addItem(QString("All"));

    drawLable->clearDraw();
    drawLable->setNewQImage(currentImage);
    drawLable->setEnabled(false);
    drawMarkDataWidget->setCurrentIndex(1);
}

void OCRControlWindow::initData()
{
    initMarkData(".", MarkDataType::OCR);
    initImageData();
}

void OCRControlWindow::initConnect()
{
    connect(markDataListWidget, &QListWidget::itemClicked, this, &OCRControlWindow::slotImageItem);
    connect(drawMarkDataWidget, &MyStackedWidget::signalsKey, this, &OCRControlWindow::slotScrollArea);
    connect(classBox, &QComboBox::currentTextChanged, this, &OCRControlWindow::slotChangeClass);
}

void OCRControlWindow::initImageList()
{
    processDataFlagList.clear();
    readMarkHistory();
}

void OCRControlWindow::loadImageData(const QString &imagePath, const QString &saveAnnotationsDir)
{
    QImageReader reader(imagePath);
    reader.setDecideFormatFromContent(true);
    if(reader.canRead())
    {
        QImage tempImg;
        if(reader.read(&tempImg))
        {
            currentImage = tempImg.copy();
            currentImagePath = imagePath;
            updateImage();
            QFileInfo imageFileInfo(currentImagePath);
            QString readJsonPath= saveAnnotationsDir + "/" + imageFileInfo.completeBaseName() + ".json";
            QFileInfo jsonFileInfo(readJsonPath);
            QList<MyObject> objects;
            if(jsonFileInfo.exists() && jsonProcess.readJSON(readJsonPath, objects) == 0)
            {

            }
            drawLable->setOjects(objects, classBox->currentText());
        }
        else
        {
            QMessageBox::information(this, tr("加载图片"), tr("读取图片失败！"));
        }
    }
    else
    {
        // QImageReader::supportedImageFormats()
        //QMessageBox::information(this, tr("加载图片"), tr("该图片加载失败, 目前支持格式：\n%1").arg());
        QMessageBox::information(this, tr("加载图片"), tr("该图片加载失败,请点击下一张图片！"));
    }

//    QImage tempImg;
//    if(tempImg.load(imagePath))
//    {
//        currentImage = tempImg.copy();
//        currentImagePath = imagePath;
//        updateImage();
//        QFileInfo imageFileInfo(currentImagePath);
//        QString readJsonPath= saveAnnotationsDir + "/" + imageFileInfo.completeBaseName() + ".json";
//        QFileInfo jsonFileInfo(readJsonPath);
//        QList<MyObject> objects;
//        if(jsonFileInfo.exists() && jsonProcess.readJSON(readJsonPath, objects) == 0)
//        {

//        }
//        drawLable->setOjects(objects, classBox->currentText());
//    }
//    else
//    {
//        QMessageBox::information(this, tr("加载图片"), tr("该图片加载失败,请点击下一张图片！"));
//    }
}

void OCRControlWindow::saveImageDataResult(const QString &saveAnnotationsDir, const QString &imagePath,
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
}

void OCRControlWindow::initImageData()
{
    this->currentImagePath = "";
}

