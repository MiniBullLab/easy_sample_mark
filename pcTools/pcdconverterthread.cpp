#include "pcdconverterthread.h"
#include <iostream>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

PCDConverterThread::PCDConverterThread()
{
    init();
}

PCDConverterThread::~PCDConverterThread()
{

}

void PCDConverterThread::run()
{
    if (isStart)
    {
        QString saveDirName = this->dirName + "/temps";
        QList<QString> pcdList = dirProcess.getDirFileName(this->dirName, suffix);
        myMakeDir(saveDirName);
        saveDirName += "/";
        if(suffix.contains("pcd") && format.contains("bin"))
        {
            pcdConvertToBin(saveDirName, pcdList);
        }
        else if(suffix.contains("ply") && format.contains("bin"))
        {
            plyConvertToBin(saveDirName, pcdList);
        }
        emit signalFinish(saveDirName);
    }
}

int PCDConverterThread::initData(const QString &fileNameDir, const QString& fileSuffix,
                                 const QString& format, int fieldsNumber)
{
    if(fieldsNumber == 3 || fieldsNumber == 4 || fieldsNumber == 6)
    {
        this->dirName = fileNameDir;
        this->suffix = fileSuffix;
        this->format = format;
        this->fieldsNumber = fieldsNumber;
        return 0;
    }
    else
    {
        return -1;
    }
}

void PCDConverterThread::startThread()
{
    isStart = true;
}

void PCDConverterThread::stopThread()
{
    isStart = false;
}

bool PCDConverterThread::myMakeDir(const QString& pathDir)
{
    QDir dir;
    if (!dir.exists(pathDir))
    {
        if (!dir.mkpath(pathDir))
        {
            std::cout << "make dir fail!" << "fileName:" << pathDir.toStdString() << std::endl;
            return false;
        }
    }
    return true;
}

void PCDConverterThread::pcdConvertToBin(const QString &saveDir, const QList<QString> &dataList)
{
    for (int i = 0; i< dataList.size(); ++i)
    {
        if (!isStart)
        {
            break;
        }
        QFileInfo fileInfo(dataList[i]);
        QString fileName = fileInfo.completeBaseName();
        QString saveFileName = saveDir + fileName + format;
        pcl::PCLPointCloud2::Ptr srcCloud(new pcl::PCLPointCloud2);
        if(pcReader.pcdRead(dataList[i].toStdString(), srcCloud) < 0)
        {
            continue;
        }
        pcWriter.savePointCloudToBin(srcCloud, saveFileName.toStdString(),
                                     this->fieldsNumber);
    }
}

void PCDConverterThread::plyConvertToBin(const QString &saveDir, const QList<QString> &dataList)
{
    for (int i = 0; i< dataList.size(); ++i)
    {
        if (!isStart)
        {
            break;
        }
        QFileInfo fileInfo(dataList[i]);
        QString fileName = fileInfo.completeBaseName();
        QString saveFileName = saveDir + fileName + format;
        pcl::PCLPointCloud2::Ptr srcCloud(new pcl::PCLPointCloud2);
        if(pcReader.plyRead(dataList[i].toStdString(), srcCloud) < 0)
        {
            continue;
        }
        pcWriter.savePointCloudToBin(srcCloud, saveFileName.toStdString(),
                                     this->fieldsNumber);
    }
}

void PCDConverterThread::init()
{
    dirName = "";
    suffix = "";
    format = "";
    fieldsNumber = 3;
    isStart = false;
}
