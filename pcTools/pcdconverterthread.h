#ifndef PCDCONVERTERTHREAD_H
#define PCDCONVERTERTHREAD_H

#include <QThread>
#include "helpers/dirprocess.h"
#include "saveData/pointcloudwriter.h"
#include "saveData/pointcloudreader.h"

class PCDConverterThread : public QThread
{
    Q_OBJECT

public:
    PCDConverterThread();
    ~PCDConverterThread();

    int initData(const QString &fileNameDir, const QString& fileSuffix,
                 const QString& format, int fieldsNumber);

    void startThread();
    void stopThread();

signals:
    void signalFinish(QString name);

public slots:

protected:
    void run();

private:

    DirProcess dirProcess;
    PointCloudWriter pcWriter;
    PointCloudReader pcReader;

    QString dirName;
    QString suffix;
    QString format;
    int fieldsNumber;

    bool isStart;

    void init();
    bool myMakeDir(const QString& pathDir);

    void pcdConvert(const QString &saveDir, const QList<QString> &dataList);
    void plyConvert(const QString &saveDir, const QList<QString> &dataList);
    void plyConvertToMesh(const QString &saveDir, const QList<QString> &dataList);

    void poissonReconstruction(const std::string savePath, const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud);
};

#endif // PCDCONVERTERTHREAD_H
