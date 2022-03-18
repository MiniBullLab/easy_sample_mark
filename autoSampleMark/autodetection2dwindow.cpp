#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "autodetection2dwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QIcon>
#include <QMenu>
#include <QDebug>

#include "helpers/dirprocess.h"
#include "sampleMarkParam/autoparamterconfig.h"

AutoDetection2DWindow::AutoDetection2DWindow(QWidget *parent)
    : QWidget(parent)
{
    initMainUI();
    initMenu();
    initConnect();
}

AutoDetection2DWindow::~AutoDetection2DWindow()
{

}

void AutoDetection2DWindow::initData()
{
    processVideoList.clear();
    historyVideo.clear();
    videoPathDir = ".";
    currentProcessVideo = "";
    currentFrameNumber = 0;
    updateProgressBar();
}

void AutoDetection2DWindow::closeEvent(QCloseEvent *event)
{
    if(sampleMarkThread.isRunning())
    {
        QMessageBox::StandardButton button;
        button = QMessageBox::question(this,tr("自动化样本标注程序"),QString(tr("自动化样本标注程序正在运行，是否退出？")),
                                     QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::No)
        {
            event->ignore();
        }
        else if(button==QMessageBox::Yes)
        {
            historyProcess.writeHistoryData(this->videoPathDir, this->historyVideo);
            sampleMarkThread.stopThread();
            sampleMarkThread.wait();
            event->accept();
            QWidget::closeEvent(event);
            emit signalCloseAutoSampleMarkWindow("mark");
        }
    }
    else
    {
        historyProcess.writeHistoryData(this->videoPathDir, this->historyVideo);
        event->accept();
        QWidget::closeEvent(event);
        emit signalCloseAutoSampleMarkWindow("mark");
    }
}

void AutoDetection2DWindow::contextMenuEvent (QContextMenuEvent * event)
{
    QMenu* popMenu = new QMenu(this);
    if(videoListWidget->itemAt(videoListWidget->mapFromGlobal(QCursor::pos())) != NULL) //如果有item则添加"修改"菜单
    {
        if (!isProcessDirBox->isChecked() && videoListWidget->currentItem()->data(Qt::UserRole).toInt() > 0)
        {
            if(currentProcessVideo.trimmed().isEmpty())
            {
                popMenu->addAction(startVideoProcessAction);
                popMenu->addAction(stopVideoProcessAction);
                startVideoProcessAction->setEnabled(true);
                stopVideoProcessAction->setEnabled(false);
            }
            else if(currentProcessVideo.startsWith(videoListWidget->currentItem()->text()))
            {
                popMenu->addAction(startVideoProcessAction);
                popMenu->addAction(stopVideoProcessAction);
                startVideoProcessAction->setEnabled(false);
                stopVideoProcessAction->setEnabled(true);
            }
        }
    }
    // 菜单出现的位置为当前鼠标的位置
    popMenu->exec(QCursor::pos());
    QWidget::contextMenuEvent(event);
}

void AutoDetection2DWindow::slotOpenDir()
{
    DirProcess dirProcess;
    this->videoPathDir = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), videoPathDir, QFileDialog::ShowDirsOnly);
    this->videoDirText->setText(this->videoPathDir);
    if(this->videoPathDir.trimmed().isEmpty() || !QDir(this->videoPathDir).exists())
    {
        qDebug() << "打开的文件路径有误:" << this->videoPathDir << endl;
    }
    else
    {

        processVideoList.clear();
        historyVideo.clear();
        QList<QString> pathList;
        pathList.clear();
        dirProcess.getDirAllFileName(this->videoPathDir, videoPostBox->currentText(), pathList);
        if (pathList.size() > 0)
        {
            historyProcess.readHistoryData(this->videoPathDir);
            initProcessVideoList(pathList);
            if(isProcessDirBox->isChecked())
            {
                startProcessButton->setEnabled(true);
                stopProcessButton->setEnabled(false);
            }
            else
            {
                startProcessButton->setEnabled(false);
                stopProcessButton->setEnabled(false);
            }
        }
        else
        {
            QMessageBox::information(this, tr("视频数据信息"), tr("%1没有%2视频").arg(this->videoPathDir).arg(videoPostBox->currentText()));
        }
        updateListBox();
    }
}

void AutoDetection2DWindow::slotStart()
{
    int errorCode = -1;
    if(!onnxNetText->text().trimmed().isEmpty())
    {
        errorCode = sampleMarkThread.initModel(selectNetBox->currentText(), onnxNetText->text());
    }
    else
    {
        errorCode = sampleMarkThread.initModel(selectNetBox->currentText(), caffeNetText->text(), caffeModelText->text());
    }
    if(errorCode < 0)
    {
        stopProcessButton->setEnabled(false);
        videoListWidget->setEnabled(false);
        centerGroundBox->setEnabled(false);
        QMessageBox::information(this, tr("检测模型初始化"), tr("检测模型初始化失败"));
    }
    else
    {
        if(isProcessDirBox->isChecked())
        {
            sampleMarkThread.initData(processVideoList, skipFrameBox->value());
            sampleMarkThread.startThread();
            sampleMarkThread.start();
            startProcessButton->setEnabled(false);
            stopProcessButton->setEnabled(true);
            centerGroundBox->setEnabled(false);
        }
        else
        {
            QList<QString> tempVideo;
            currentProcessVideo = videoListWidget->currentItem()->text();
            tempVideo.append(currentProcessVideo);
            sampleMarkThread.initData(tempVideo, skipFrameBox->value());
            sampleMarkThread.startThread();
            sampleMarkThread.start();
            centerGroundBox->setEnabled(false);
        }
    }
}

void AutoDetection2DWindow::slotStop()
{
    sampleMarkThread.stopThread();
    startProcessButton->setEnabled(false);
    stopProcessButton->setEnabled(false);
    centerGroundBox->setEnabled(false);
}

void AutoDetection2DWindow::slotIsProcessDir(bool isChecked)
{
    if(processVideoList.size() > 0)
    {
        if(isChecked)
        {
            startProcessButton->setEnabled(true);
            stopProcessButton->setEnabled(false);
        }
        else
        {
            startProcessButton->setEnabled(false);
            stopProcessButton->setEnabled(false);
        }
    }
}

void AutoDetection2DWindow::slotVideoInit(int allFrameCount)
{
    this->videoProgressBar->setRange(0, allFrameCount);
}

void AutoDetection2DWindow::slotVideoCurrentFrame(int number)
{
    this->currentFrameNumber = number;
    updateProgressBar();
}

void AutoDetection2DWindow::slotVideoFinish(QString videoInfo)
{
    commandText->append(videoInfo);
    if(videoInfo.startsWith("Video"))
    {
        QString videoName = videoInfo.mid(6);
        historyVideo.append(videoName);
        processVideoList.removeOne(videoName);
        qDebug() << "video:" << videoName << endl;
        updateListBox();
    }
    currentFrameNumber = 0;
    updateProgressBar();
}

void AutoDetection2DWindow::slotFinishAll()
{
    centerGroundBox->setEnabled(true);
    if(isProcessDirBox->isChecked()&& processVideoList.size() > 0)
    {
        startProcessButton->setEnabled(true);
        stopProcessButton->setEnabled(false);
    }
    else
    {
        startProcessButton->setEnabled(false);
        stopProcessButton->setEnabled(false);
    }
    currentProcessVideo = "";
    historyProcess.writeHistoryData(this->videoPathDir, this->historyVideo);
}

void AutoDetection2DWindow::slotSelectDL()
{
    if(selectNetBox->currentText() == "ssd")
    {
        caffeNetButton->setEnabled(true);
        caffeModelButton->setEnabled(true);
        onnxNetButton->setEnabled(false);
    }
    else if(selectNetBox->currentText() == "yolov5")
    {
        caffeNetButton->setEnabled(false);
        caffeModelButton->setEnabled(false);
        onnxNetButton->setEnabled(true);
    }
}

void AutoDetection2DWindow::slotSelectCaffeNet()
{
    QString name = QFileDialog::getOpenFileName(this,tr("选择caffe网络"),".","caffe files(*.prototxt *.*)");
    if(!name.trimmed().isEmpty())
    {
        this->caffeNetText->setText(name);
    }
}

void AutoDetection2DWindow::slotSelectCaffeModel()
{
    QString name = QFileDialog::getOpenFileName(this,tr("选择caffe权重"),".","caffe files(*.caffemodel *.*)");
    if(!name.trimmed().isEmpty())
    {
        this->caffeModelText->setText(name);
    }
}

void AutoDetection2DWindow::slotSelectOnnxModel()
{
    QString name = QFileDialog::getOpenFileName(this,tr("选择onnx模型"),".","onnx files(*.onnx *.*)");
    if(!name.trimmed().isEmpty())
    {
        this->onnxNetText->setText(name);
    }
}

void AutoDetection2DWindow::initProcessVideoList(const QList<QString>& pathList)
{
    for(int index = 0; index < pathList.size(); index++)
    {
        if(historyVideo.size() > 0)
        {
            if(historyVideo.indexOf(pathList[index]) == -1)
            {
                processVideoList.append(pathList[index]);
            }
        }
        else
        {
            processVideoList.append(pathList[index]);
        }
    }
}

void AutoDetection2DWindow::updateListBox()
{
    int index1 = 0;
    int index2 = 0;
    videoListWidget->clear();
    for(index1 = 0; index1 < historyVideo.size(); index1++)
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/qss_icons/style/rc/checkbox_checked_disabled.png"),
                                                    historyVideo[index1]);
        item->setData(Qt::UserRole, 0);
        videoListWidget->insertItem(index1, item);
    }
    for(index2 = 0; index2 < processVideoList.size(); index2++)
    {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/qss_icons/style/rc/checkbox_checked_focus.png"),
                                                    processVideoList[index2]);
        item->setData(Qt::UserRole, 1);
        videoListWidget->insertItem(index2 + index1, item);
        //videoListWidget->closePersistentEditor(item);
    }
    if(processVideoList.size() >0)
    {
        videoListWidget->setCurrentRow(index1);
    }
}

void AutoDetection2DWindow::updateProgressBar()
{
    videoProgressBar->setValue(currentFrameNumber);
}

void AutoDetection2DWindow::initMainUI()
{
    selectNetLabel = new QLabel(tr("选择深度学习模型："));
    selectNetBox = new QComboBox();
    selectNetBox->addItem("ssd");
    selectNetBox->addItem("yolov5");

    QHBoxLayout *topLayout0 = new QHBoxLayout();
    topLayout0->setSpacing(30);
    topLayout0->addWidget(selectNetLabel);
    topLayout0->addWidget(selectNetBox);

    caffeNetLabel = new QLabel(tr("caffe网络："));
    caffeNetText = new QLineEdit();
    caffeNetText->setReadOnly(true);
    caffeNetButton = new QPushButton(tr("选择caffe网络"));

    caffeModelLabel = new QLabel(tr("caffe权重："));
    caffeModelText = new QLineEdit();
    caffeModelText->setReadOnly(true);
    caffeModelButton = new QPushButton(tr("选择caffe权重"));

    QHBoxLayout *topLayout1 = new QHBoxLayout();
    topLayout1->setSpacing(10);
    topLayout1->addWidget(caffeNetLabel);
    topLayout1->addWidget(caffeNetText);
    topLayout1->addWidget(caffeNetButton);
    topLayout1->addWidget(caffeModelLabel);
    topLayout1->addWidget(caffeModelText);
    topLayout1->addWidget(caffeModelButton);

    onnxNetLabel = new QLabel(tr("onnx模型："));
    onnxNetText = new QLineEdit();
    onnxNetText->setReadOnly(true);
    onnxNetButton = new QPushButton(tr("选择onnx模型"));

    QHBoxLayout *topLayout2 = new QHBoxLayout();
    topLayout2->setSpacing(30);
    topLayout2->addWidget(onnxNetLabel);
    topLayout2->addWidget(onnxNetText);
    topLayout2->addWidget(onnxNetButton);

    QVBoxLayout *topLayout = new QVBoxLayout();
    topLayout->setSpacing(10);
    topLayout->addLayout(topLayout0);
    topLayout->addLayout(topLayout1);
    topLayout->addLayout(topLayout2);

    slotSelectDL();

    netGroundBox = new QGroupBox();
    netGroundBox->setLayout(topLayout);

    isProcessDirBox = new QCheckBox(tr("是否批量处理"));
    isProcessDirBox->setChecked(true);

    videoPostLabel = new QLabel(tr("处理视频格式："));
    videoPostBox = new QComboBox();
    videoPostBox->addItem(tr("*.mov"));
    videoPostBox->addItem(tr("*.avi"));
    videoPostBox->addItem(tr("*.mp4"));
    videoPostBox->addItem(tr("*.mpg"));
    videoPostBox->addItem(tr("*.*"));

    saveImagePostLabel = new QLabel(tr("保存图片格式："));
    saveImagePostBox = new QComboBox();
    saveImagePostBox->addItem(".png");
    saveImagePostBox->addItem(".jpg");
    saveImagePostBox->addItem(".gif");

    skipFrameLabel = new QLabel(tr("视频间隔处理帧数："));
    skipFrameBox = new QSpinBox();
    skipFrameBox->setMinimum(1);
    skipFrameBox->setValue(50);
    skipFrameBox->setMaximum(1000);

    QGridLayout *centerTopLayout0 = new QGridLayout();
    centerTopLayout0->setSpacing(30);
    centerTopLayout0->addWidget(isProcessDirBox, 0, 0, 1, 1);
    centerTopLayout0->addWidget(videoPostLabel, 1, 0, 1, 1);
    centerTopLayout0->addWidget(videoPostBox, 1, 1, 1, 1);
    centerTopLayout0->addWidget(saveImagePostLabel, 1, 2, 1, 1);
    centerTopLayout0->addWidget(saveImagePostBox, 1, 3, 1, 1);
    centerTopLayout0->addWidget(skipFrameLabel, 2, 0, 1, 1);
    centerTopLayout0->addWidget(skipFrameBox, 2, 1, 1, 1);

    videoDirLabel = new QLabel(tr("视频目录："));
    videoDirText = new QLineEdit();
    videoDirText->setReadOnly(true);
    openVideoDirButton = new QPushButton(tr("打开视频目录"));

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(20);
    hLayout->addWidget(videoDirLabel);
    hLayout->addWidget(videoDirText);
    hLayout->addWidget(openVideoDirButton);

    QVBoxLayout *centerTopLayout1 = new QVBoxLayout();
    centerTopLayout1->addSpacing(20);
    centerTopLayout1->addLayout(centerTopLayout0);
    centerTopLayout1->addLayout(hLayout);

    centerGroundBox = new QGroupBox(tr("参数配置"));
    centerGroundBox->setLayout(centerTopLayout1);

    startProcessButton = new QPushButton(tr("开始处理"));
    stopProcessButton = new QPushButton(tr("结束处理"));
    startProcessButton->setEnabled(false);
    stopProcessButton->setEnabled(false);

    QHBoxLayout *hLayout1 = new QHBoxLayout();
    hLayout1->setSpacing(200);
    hLayout1->addWidget(startProcessButton);
    hLayout1->addWidget(stopProcessButton);

    videoProgressLabel = new QLabel(tr("视频处理进度："));
    videoProgressBar = new QProgressBar();

    QHBoxLayout *hLayout2 = new QHBoxLayout();
    hLayout2->setSpacing(20);
    hLayout2->addWidget(videoProgressLabel);
    hLayout2->addWidget(videoProgressBar);

    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setSpacing(10);
    centerLayout->addWidget(netGroundBox);
    centerLayout->addWidget(centerGroundBox);
    centerLayout->addLayout(hLayout1);
    centerLayout->addLayout(hLayout2);

    videoListWidget = new QListWidget();

    QHBoxLayout *allLayout = new QHBoxLayout();
    allLayout->addLayout(centerLayout);
    allLayout->addWidget(videoListWidget);
    allLayout->setStretchFactor(centerLayout, 3);
    allLayout->setStretchFactor(videoListWidget, 1);

    commandText = new MyTextBrowser();
    // commandText->setFixedHeight(50);
    commandText->setReadOnly(true);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(10); //设置这个对话框的边距
    mainLayout->setSpacing(10);  //设置各个控件之间的边距
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(allLayout);
    mainLayout->addWidget(commandText);
    mainLayout->setStretchFactor(allLayout, 3);
    mainLayout->setStretchFactor(commandText, 1);

    this->setLayout(mainLayout);
    //this->setMaximumSize(700,520);
    this->setMinimumSize(800, 700);
    this->setWindowTitle(tr("自动化样本标注"));
}

void AutoDetection2DWindow::initMenu()
{
    startVideoProcessAction = new QAction(tr("开始处理"), this);
    stopVideoProcessAction = new QAction(tr("停止处理"), this);
}

void AutoDetection2DWindow::initConnect()
{
    connect(openVideoDirButton, &QPushButton::clicked, this,&AutoDetection2DWindow::slotOpenDir);
    connect(startProcessButton, &QPushButton::clicked, this,&AutoDetection2DWindow::slotStart);
    connect(stopProcessButton, &QPushButton::clicked, this,&AutoDetection2DWindow::slotStop);
    connect(isProcessDirBox, &QCheckBox::clicked, this, &AutoDetection2DWindow::slotIsProcessDir);
    connect(&sampleMarkThread, &AutoSampleMarkThread::signalVideoFinish, this, &AutoDetection2DWindow::slotVideoFinish);
    connect(&sampleMarkThread, &AutoSampleMarkThread::signalFinishAll, this, &AutoDetection2DWindow::slotFinishAll);
    connect(&sampleMarkThread, &AutoSampleMarkThread::signalCurrentFrame, this, &AutoDetection2DWindow::slotVideoCurrentFrame);
    connect(&sampleMarkThread, &AutoSampleMarkThread::signalVideoInit, this, &AutoDetection2DWindow::slotVideoInit);
    connect(startVideoProcessAction, &QAction::triggered, this, &AutoDetection2DWindow::slotStart);
    connect(stopVideoProcessAction, &QAction::triggered, this, &AutoDetection2DWindow::slotStop);

    connect(selectNetBox, &QComboBox::currentTextChanged, this, &AutoDetection2DWindow::slotSelectDL);
    connect(caffeNetButton, &QPushButton::clicked, this, &AutoDetection2DWindow::slotSelectCaffeNet);
    connect(caffeModelButton, &QPushButton::clicked, this, &AutoDetection2DWindow::slotSelectCaffeModel);
    connect(onnxNetButton, &QPushButton::clicked, this, &AutoDetection2DWindow::slotSelectOnnxModel);
}
