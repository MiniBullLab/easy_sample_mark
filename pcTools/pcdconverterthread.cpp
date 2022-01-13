#include "pcdconverterthread.h"
#include <iostream>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QDebug>

#include <pcl/filters/passthrough.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/normal_3d_omp.h>
#include <pcl/surface/gp3.h>
#include <pcl/surface/poisson.h>

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
        else if(suffix.contains("ply") && format.contains("txt"))
        {
            std::cout << "convert txt" << std::endl;
            plyConvertToTxt(saveDirName, pcdList);
        }
        else if(suffix.contains("ply") && format.contains("mesh"))
        {
            plyConvertToMesh(saveDirName, pcdList);
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

void PCDConverterThread::plyConvertToTxt(const QString &saveDir, const QList<QString> &dataList)
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
        pcWriter.savePointCloudToTxt(srcCloud, saveFileName.toStdString(),
                                     this->fieldsNumber);
    }
}

void PCDConverterThread::plyConvertToMesh(const QString &saveDir, const QList<QString> &dataList)
{
    for (int i = 0; i< dataList.size(); ++i)
    {
        if (!isStart)
        {
            break;
        }
        QFileInfo fileInfo(dataList[i]);
        QString fileName = fileInfo.completeBaseName();
        QString saveFileName = saveDir + fileName + ".ply";
        pcl::PCLPointCloud2::Ptr srcCloud(new pcl::PCLPointCloud2);
        if(pcReader.plyRead(dataList[i].toStdString(), srcCloud) < 0)
        {
            continue;
        }
        pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>);
        pcl::fromPCLPointCloud2(*srcCloud, *cloud);
        poissonReconstruction(saveFileName.toStdString(), cloud);
    }
}

void PCDConverterThread::poissonReconstruction(const std::string savePath, const pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud)
{
      pcl::PointCloud<pcl::PointXYZRGB>::Ptr filtered(new pcl::PointCloud<pcl::PointXYZRGB>());
      pcl::PassThrough<pcl::PointXYZRGB> filter;
      filter.setInputCloud(cloud);
      filter.filter(*filtered);
      std::cout << "passthrough filter complete" << std::endl;

      std::cout << "begin normal estimation" << std::endl;
      pcl::NormalEstimationOMP<pcl::PointXYZRGB, pcl::Normal> ne;//计算点云法向
      ne.setNumberOfThreads(8);//设定临近点
      ne.setInputCloud(filtered);
      ne.setRadiusSearch(0.01);//设定搜索半径
      Eigen::Vector4f centroid;
      compute3DCentroid(*filtered, centroid);//计算点云中心
      ne.setViewPoint(centroid[0], centroid[1], centroid[2]);//将向量计算原点置于点云中心

      pcl::PointCloud<pcl::Normal>::Ptr cloud_normals(new pcl::PointCloud<pcl::Normal>());
      ne.compute(*cloud_normals);
      std::cout << "normal estimation complete" << std::endl;
      std::cout << "reverse normals' direction" << std::endl;

      //将法向量反向
      for(size_t i = 0; i < cloud_normals->size(); ++i)
      {
        cloud_normals->points[i].normal_x *= -1;
        cloud_normals->points[i].normal_y *= -1;
        cloud_normals->points[i].normal_z *= -1;
      }

      //融合RGB点云和法向
      std::cout << "combine points and normals" << std::endl;
      pcl::PointCloud<pcl::PointXYZRGBNormal>::Ptr cloud_smoothed_normals(new pcl::PointCloud<pcl::PointXYZRGBNormal>());
      concatenateFields(*filtered, *cloud_normals, *cloud_smoothed_normals);

      //泊松重建
      std::cout << "begin poisson reconstruction" << std::endl;
      pcl::Poisson<pcl::PointXYZRGBNormal> poisson;
      poisson.setDegree(2);
      poisson.setDepth(10);
      poisson.setSolverDivide(8);
      poisson.setIsoDivide(8);

      poisson.setConfidence(false);
      poisson.setManifold(true);
      poisson.setOutputPolygons(true);

      poisson.setInputCloud(cloud_smoothed_normals);
      PolygonMesh mesh;
      poisson.reconstruct(mesh);

      std::cout << "finish poisson reconstruction" << std::endl;

      //给mesh染色
      pcl::PointCloud<pcl::PointXYZRGB> cloud_color_mesh;
      pcl::fromPCLPointCloud2(mesh.cloud, cloud_color_mesh);

      pcl::KdTreeFLANN<pcl::PointXYZRGB> kdtree;
      kdtree.setInputCloud (cloud);
      // K nearest neighbor search
      int K = 5;
      std::vector<int> pointIdxNKNSearch(K);
      std::vector<float> pointNKNSquaredDistance(K);
      for(int i=0;i<cloud_color_mesh.points.size();++i)
      {
         uint8_t r = 0;
         uint8_t g = 0;
         uint8_t b = 0;
         float dist = 0.0;
         int red = 0;
         int green = 0;
         int blue = 0;
         uint32_t rgb;

         if ( kdtree.nearestKSearch (cloud_color_mesh.points[i], K, pointIdxNKNSearch, pointNKNSquaredDistance) > 0 )
         {
            for (int j = 0; j < pointIdxNKNSearch.size (); ++j)
            {

               r = cloud->points[ pointIdxNKNSearch[j] ].r;
               g = cloud->points[ pointIdxNKNSearch[j] ].g;
               b = cloud->points[ pointIdxNKNSearch[j] ].b;

               red += int(r);
               green += int(g);
               blue += int(b);
               dist += 1.0/pointNKNSquaredDistance[j];

               std::cout<<"red: "<<int(r)<<std::endl;
               std::cout<<"green: "<<int(g)<<std::endl;
               std::cout<<"blue: "<<int(b)<<std::endl;
               std::cout << "dis:" << dist << std::endl;
            }
         }

         cloud_color_mesh.points[i].r = int(red/pointIdxNKNSearch.size ()+0.5);
         cloud_color_mesh.points[i].g = int(green/pointIdxNKNSearch.size ()+0.5);
         cloud_color_mesh.points[i].b = int(blue/pointIdxNKNSearch.size ()+0.5);


      }
      toPCLPointCloud2(cloud_color_mesh, mesh.cloud);
      io::savePLYFile(savePath, mesh);
}

void PCDConverterThread::init()
{
    dirName = "";
    suffix = "";
    format = "";
    fieldsNumber = 3;
    isStart = false;
}
