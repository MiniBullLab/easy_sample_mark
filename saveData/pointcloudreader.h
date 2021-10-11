#ifndef POINTCLOUDREADER_H
#define POINTCLOUDREADER_H

#include <string>
#include <iostream>
#include <fstream>

#ifndef PCL_NO_PRECOMPILE
#define PCL_NO_PRECOMPILE
#endif

#include <pcl/common/common.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>


class PointCloudReader
{
public:
    PointCloudReader();
    ~PointCloudReader();

    int pcdRead(const std::string &pcdPath, pcl::PCLPointCloud2::Ptr &resultCloud);
    int pcdRead(const std::string &pcdPath, pcl::PCLPointCloud2::Ptr &resultCloud,
                Eigen::Vector4f &origin, Eigen::Quaternionf &orientation);
    int plyRead(const std::string &plyPath, pcl::PCLPointCloud2::Ptr &resultCloud);
    int xyzBinRead(const std::string &binPath, pcl::PointCloud<pcl::PointXYZI>::Ptr &resultCloud);
    int xyziBinRead(const std::string &binPath, pcl::PointCloud<pcl::PointXYZI>::Ptr &resultCloud);
    int rbgBinRead(const std::string &binPath, pcl::PointCloud<pcl::PointXYZRGB>::Ptr &resultCloud);

private:
    pcl::PCDReader pcdReader;
    pcl::PLYReader plyReader;
};

#endif // POINTCLOUDREADER_H
