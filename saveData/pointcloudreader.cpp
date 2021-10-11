#include "pointcloudreader.h"


PointCloudReader::PointCloudReader()
{

}

PointCloudReader::~PointCloudReader()
{

}

int PointCloudReader::pcdRead(const std::string &pcdPath, pcl::PCLPointCloud2::Ptr &resultCloud)
{
    int ret = 0;
    resultCloud.reset();
    try {
        if(pcdReader.read(pcdPath, *resultCloud) < 0)
        {
            std::cout << "open " << pcdPath << " faild!" << std::endl;
            ret = -1;
        }
    } catch (std::exception e) {
       std::cout << e.what() << "|open " << pcdPath << " faild!" << std::endl;;
       ret = -1;
    }
    return ret;
}

int PointCloudReader::pcdRead(const std::string &pcdPath, pcl::PCLPointCloud2::Ptr &resultCloud,
                              Eigen::Vector4f &origin, Eigen::Quaternionf &orientation)
{
    int ret = 0;
    int version;
//    pcl::PointCloud<pcl::PointXYZI>::Ptr currentCloud(new pcl::PointCloud<pcl::PointXYZI>);
//    if(pcl::io::loadPCDFile(pcdFilePath.toStdString(), *currentCloud) == -1)
//    {
//        return -1;
//    }
    resultCloud.reset(new pcl::PCLPointCloud2);
    try {
        if(pcdReader.read(pcdPath, *resultCloud, origin, orientation, version) < 0)
        {
            std::cout << "open " << pcdPath << " faild!" << std::endl;
            ret = -1;
        }
    } catch (std::exception e) {
       std::cout << e.what() << "|open " << pcdPath << " faild!" << std::endl;;
       ret = -1;
    }
    return ret;
}

int PointCloudReader::plyRead(const std::string &plyPath, pcl::PCLPointCloud2::Ptr &resultCloud)
{
    int ret = 0;
    resultCloud.reset(new pcl::PCLPointCloud2);
    try {
        if(plyReader.read(plyPath, *resultCloud) < 0)
        {
            std::cout << "open " << plyPath << " faild!" << std::endl;
            ret = -1;
        }
    } catch (std::exception e) {
       std::cout << e.what() << "|open " << plyPath << " faild!" << std::endl;
       ret = -1;
    }
    return ret;
}

int PointCloudReader::xyzBinRead(const std::string &binPath, pcl::PointCloud<pcl::PointXYZI>::Ptr &resultCloud)
{
    float data[3];
    std::ifstream inFile(binPath, std::ios::in|std::ios::binary);
    resultCloud->clear();
    if(!inFile)
    {
        std::cout << binPath << " open error!" << std::endl;
        return -1;
    }
    while(inFile.read(reinterpret_cast<char*>(data), sizeof(data)))
    {
        pcl::PointXYZI point;
        point.x = data[0];
        point.y = data[1];
        point.z = data[2];
        point.intensity = 255;
        resultCloud->push_back(point);
    }
    inFile.close();
    return 0;
}

int PointCloudReader::xyziBinRead(const std::string &binPath, pcl::PointCloud<pcl::PointXYZI>::Ptr &resultCloud)
{
    float data[4];
    std::ifstream inFile(binPath, std::ios::in|std::ios::binary);
    resultCloud->clear();
    if(!inFile)
    {
        std::cout << binPath << " open error!" << std::endl;
        return -1;
    }
    while(inFile.read(reinterpret_cast<char*>(data), sizeof(data)))
    {
        pcl::PointXYZI point;
        point.x = data[0];
        point.y = data[1];
        point.z = data[2];
        point.intensity = data[3];
        resultCloud->push_back(point);
    }
    inFile.close();
    return 0;
}

int PointCloudReader::rbgBinRead(const std::string &binPath, pcl::PointCloud<pcl::PointXYZRGB>::Ptr &resultCloud)
{
    float data[6];
    std::ifstream inFile(binPath, std::ios::in|std::ios::binary);
    resultCloud->clear();
    if(!inFile)
    {
        std::cout << binPath << " open error!" << std::endl;
        return -1;
    }
    while(inFile.read(reinterpret_cast<char*>(data), sizeof(data)))
    {
        pcl::PointXYZRGB point;
        point.x = data[0];
        point.y = data[1];
        point.z = data[2];
        point.r = static_cast<unsigned char>(data[3]);
        point.g = static_cast<unsigned char>(data[4]);
        point.b = static_cast<unsigned char>(data[5]);
        resultCloud->push_back(point);
    }
    inFile.close();
    return 0;
}
