#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "autosamplemarkthread.h"
#include <QFileInfo>
#include <QDir>
#include <QImage>

#include <iostream>

#include "sampleMarkParam/manualparamterconfig.h"
#include "sampleMarkParam/autoparamterconfig.h"

AutoSampleMarkThread::AutoSampleMarkThread()
{
    init();
}

AutoSampleMarkThread::~AutoSampleMarkThread()
{

}

int AutoSampleMarkThread::initModel(const QString &modelName, const QString &modelPath, const QString &modelBinary)
{
    int errorCode = -1;
    if(modelName == "ssd")
    {
        detector.reset(new SSDector());
        errorCode = detector->initModel(modelPath.toStdString(),
                                        modelBinary.toStdString());
    }
    else if(modelName == "yolov5")
    {
        std::cout << "yolov5" << std::endl;
        detector.reset(new YoloV5Dector());
        errorCode = detector->initModel(modelPath.toStdString(),
                                        modelBinary.toStdString());
    }
    return errorCode;
}

void AutoSampleMarkThread::initData(const QList<QString> &videoList, const int skipFrameCount, const QString post)
{
    std::map<int, std::string> tempLabels;
    this->videoList = videoList;
    this->skipFrameCount = skipFrameCount;
    this->imagePost = post;
    for(QMap<int, QString>::const_iterator iterator = AutoParamterConfig::getModelLabels().constBegin(); \
        iterator != AutoParamterConfig::getModelLabels().constEnd(); ++iterator)
    {
        tempLabels[iterator.key()] = iterator.value().toStdString();
    }
    this->detector->initDetectorParameters(AutoParamterConfig::getInpuDataWidth(), AutoParamterConfig::getInpuDataHeight(),
                                           AutoParamterConfig::getThreshold(), tempLabels);
}

void AutoSampleMarkThread::startThread()
{
    isStart = true;
}

void AutoSampleMarkThread::stopThread()
{
    isStart = false;
}

void AutoSampleMarkThread::run()
{
    QString processInfo = "";
    int errorCode = 0;
    int count = videoList.size();
    for(int index = 0; index < count; index++)
    {
        errorCode = videoProcess->openVideo(videoList[index]);
        if(errorCode == 0 && videoProcess->isOpen())
        {
            videoWidth = videoProcess->getSize().width;
            videoHeight = videoProcess->getSize().height;
            emit signalVideoInit(videoProcess->getFrameCount());
            markVideo(videoList[index], processInfo);
        }
        else
        {
            processInfo = QString("Error:%1 can not open!").arg(videoList[index]);
        }
        emit signalVideoFinish(processInfo);
        videoProcess->closeVideo();
        if(!isStart)
        {
            break;
        }
    }
    isStart = false;
    emit signalFinishAll();
}

void AutoSampleMarkThread::markVideo(const QString& videoPath, QString& processInfo)
{
    int errorCode = 0;
    cv::Mat frame;
    int stopFrame = videoProcess->getFrameCount();
    int currentFrame = videoProcess->getFramePosition() - 1;
    QFileInfo fileInfo(videoPath);
    QString path = fileInfo.path();
    QString videoName = fileInfo.completeBaseName();
    QString saveImageDir = path + "/" + videoName + "_JPEGImages";
    QString saveJsonDir = path + "/" + "Annotations";
    QDir makeDir;
    int widthCount = QString::number(videoProcess->getFrameCount()).count() + 1;
    if(!makeDir.exists(saveImageDir))
    {
        if(!makeDir.mkdir(saveImageDir))
        {
            std::cout << "make dir fail!" << std::endl;
        }
    }
    if(!makeDir.exists(saveJsonDir))
    {
        if(!makeDir.mkdir(saveJsonDir))
        {
            std::cout << "make dir fail!" << std::endl;
        }
    }
    while(currentFrame < stopFrame)
    {
        if(isStart)
        {
            errorCode = videoProcess->readFrame(frame);
            if(errorCode == 0)
            {
                if(currentFrame % this->skipFrameCount == 0)
                {
                    std::vector<Detect2dBox> objectRect;
                    QList<MyObject> objects;
                    detector->processDetect(frame, objectRect);
                    QString saveName = videoName + QString("_%1").arg(currentFrame, widthCount, 10, QLatin1Char('0'));
                    QString saveJsonPath = saveJsonDir + QString("/%1.json").arg(saveName);
                    QString saveImagePath = saveImageDir + "/" + saveName + imagePost;
                    objects = toMyObjects(objectRect);
                    jsonProcess.createJSON(saveJsonPath, saveImagePath, videoWidth, videoHeight, objects);
                    // xmlCreator.createXML(savXmlPath, saveImagePath, videoWidth, videoHeight, objects);
                    saveImage(saveImagePath, frame);
                    emit signalCurrentFrame(currentFrame + 1);
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
        currentFrame = videoProcess->getFramePosition() - 1;
    }

    if((currentFrame + 1) == stopFrame)
    {
        processInfo = "Video:" + videoPath;
    }
    else
    {
        if(isStart)
        {
            processInfo = QString("Error: %1 read frame error!").arg(videoPath);
        }
        else
        {
            processInfo = "Stop!";
        }
    }
}

void AutoSampleMarkThread::saveImage(const QString& imagePath, const cv::Mat &frame)
{
    cv::Mat tempMat;
    if(frame.empty())
    {
        return;
    }
    cv::cvtColor(frame, tempMat, cv::COLOR_BGR2RGB);//Qt中支持的是RGB图像, OpenCV中支持的是BGR
    QImage image = QImage((uchar*)(tempMat.data), tempMat.cols, tempMat.rows, QImage::Format_RGB888);
    if(!image.save(imagePath))
    {
        std::cout << "save image fail!" << std::endl;
    }
}

QList<MyObject> AutoSampleMarkThread::toMyObjects(const std::vector<Detect2dBox> &objectRect)
{
    QList<MyObject> objects;
    int count = static_cast<int>(objectRect.size());
    objects.clear();
    for(int index = 0; index < count; index++)
    {
        MyObject object;
        const Detect2dBox tempObject = objectRect[index];
        const float width = tempObject.maxX - tempObject.minX;
        const float height = tempObject.maxY - tempObject.minY;
        // std::cout << "width:" << width << "height:" << height << std::endl;
        if(width >= ManualParamterConfig::getMinWidth() && height >= ManualParamterConfig::getMinHeight())
        {
            object.setBox(QRect(QPoint(tempObject.minX, tempObject.minY), \
                                QPoint(tempObject.maxX, tempObject.maxY)));
            object.setObjectClass(QString::fromStdString(tempObject.objectName));
            object.setShapeType(ShapeType::RECT_SHAPE);
            objects.push_back(object);
        }
    }
    return objects;
}

void AutoSampleMarkThread::init()
{
    isStart = false;
    skipFrameCount = 1;
    videoWidth = 640;
    videoHeight = 480;

    imagePost = ".png";

    videoList.clear();

    videoProcess = std::shared_ptr<VideoProcess>(new VideoProcess());
    detector = nullptr;
}
