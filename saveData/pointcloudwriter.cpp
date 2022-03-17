#include "pointcloudwriter.h"

PointCloudWriter::PointCloudWriter()
{
    init();
}

PointCloudWriter::~PointCloudWriter()
{

}

int PointCloudWriter::savePointCloudToBin(const pcl::PCLPointCloud2::Ptr& srcCloud,
                                          const std::string &fileNamePath, const int number)
{
    if(number == 3)
    {
        pcl::PointCloud<pcl::PointXYZ> cloud;
        pcl::fromPCLPointCloud2(*srcCloud, cloud);
        if(cloud.size() > 0)
        {
            std::ofstream outF(fileNamePath, std::ios::binary);
            for(const pcl::PointXYZ &point: cloud.points)
            {
                float data[3];
                data[0] = point.x;
                data[1] = point.y;
                data[2] = point.z;
                outF.write(reinterpret_cast<char*>(data), sizeof(data));
            }
            outF.close();
        }
    }
    else if(number == 4)
    {
        pcl::PointCloud<pcl::PointXYZI> cloud;
        pcl::fromPCLPointCloud2(*srcCloud, cloud);
        if(cloud.size() > 0)
        {
            std::ofstream outF(fileNamePath, std::ios::binary);
            for(const pcl::PointXYZI &point: cloud.points)
            {
                float data[4];
                data[0] = point.x;
                data[1] = point.y;
                data[2] = point.z;
                data[3] = point.intensity;
                outF.write(reinterpret_cast<char*>(data), sizeof(data));
            }
            outF.close();
        }
    }
    else if(number == 6)
    {
        pcl::PointCloud<pcl::PointXYZRGB> cloud;
        pcl::fromPCLPointCloud2(*srcCloud, cloud);
        if(cloud.size() > 0)
        {
            std::ofstream outF(fileNamePath, std::ios::binary);
            for(const pcl::PointXYZRGB &point: cloud.points)
            {
                float data[4];
                data[0] = point.x;
                data[1] = point.y;
                data[2] = point.z;
                data[3] = point.rgb;
                outF.write(reinterpret_cast<char*>(data), sizeof(data));
            }
            outF.close();
        }
    }
    else
    {
        std::cout << number << " point cloud fields error!" << std::endl;
    }
    return 0;
}

int PointCloudWriter::savePointCloudToTxt(const pcl::PCLPointCloud2::Ptr& srcCloud,
                                          const std::string &fileNamePath, const int number)
{
    if(number == 3)
    {
        pcl::PointCloud<pcl::PointXYZ> cloud;
        pcl::fromPCLPointCloud2(*srcCloud, cloud);
        if(cloud.size() > 0)
        {
            std::ofstream outF(fileNamePath);
            for(const pcl::PointXYZ &point: cloud.points)
            {
                outF << point.x << " " << point.y << " " << point.z << std::endl;
            }
            outF.close();
        }
    }
    else if(number == 4)
    {
        pcl::PointCloud<pcl::PointXYZI> cloud;
        pcl::fromPCLPointCloud2(*srcCloud, cloud);
        if(cloud.size() > 0)
        {
            std::ofstream outF(fileNamePath, std::ios::binary);
            for(const pcl::PointXYZI &point: cloud.points)
            {
                outF << point.x << " " << point.y << " " << point.z << " " << point.intensity << std::endl;
            }
            outF.close();
        }
    }
    else if(number == 6)
    {
        pcl::PointCloud<pcl::PointXYZRGB> cloud;
        pcl::fromPCLPointCloud2(*srcCloud, cloud);
        if(cloud.size() > 0)
        {
            std::ofstream outF(fileNamePath, std::ios::binary);
            for(const pcl::PointXYZRGB &point: cloud.points)
            {
                uint32_t rgb = *reinterpret_cast<const int*>(&point.rgb);
                uint8_t r = (rgb >> 16) & 0x0000ff;
                uint8_t g = (rgb >> 8)  & 0x0000ff;
                uint8_t b = (rgb)       & 0x0000ff;
                outF << point.x << " " << point.y << " " << point.z << " " << (int)r << " " << (int)g << " " << (int)b << std::endl;
            }
            outF.close();
        }
    }
    else
    {
        std::cout << number << " point cloud fields error!" << std::endl;
    }
    return 0;
}

int PointCloudWriter::savePointCloudToPCD(const pcl::PCLPointCloud2::Ptr& srcCloud,
                        const std::string &fileNamePath)
{
    return 0;
}

void PointCloudWriter::init()
{

}
