#ifndef POINTCLOUDWRITER_H
#define POINTCLOUDWRITER_H

#include <string>
#include <iostream>

#ifndef PCL_NO_PRECOMPILE
#define PCL_NO_PRECOMPILE
#endif

#include <pcl/common/common.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>

class PointCloudWriter
{
public:
    PointCloudWriter();
    ~PointCloudWriter();

    int savePointCloudToBin(const pcl::PCLPointCloud2::Ptr& srcCloud,
                            const std::string &fileNamePath,
                            const int number);
    int savePointCloudToPCD(const pcl::PCLPointCloud2::Ptr& srcCloud,
                            const std::string &fileNamePath);

private:
    void init();
};

#endif // POINTCLOUDWRITER_H
