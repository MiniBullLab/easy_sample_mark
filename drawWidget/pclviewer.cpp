#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "pclviewer.h"
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <boost/bind.hpp>
#include"sampleMarkParam/pointcloudparamterconfig.h"

PCLViewer::PCLViewer(QWidget *parent) : QVTKWidget(parent)
{
    initData();
    initConnect();
}

PCLViewer::~PCLViewer()
{

}

void PCLViewer::setIsSelect(bool isSelect)
{
    this->isSelect = isSelect;
}

int PCLViewer::setNewPointCloud(const QString &pcdFilePath)
{
    int errorCode = 0;
    if(PointCloudParamterConfig::getFileType() == PointCloudFileType::PCD_FILE)
    {
        errorCode = pcReader.pcdRead(pcdFilePath.toStdString(), srcCloud,
                                     origin, orientation);
        dataFlag = 0;
        drawTypeColorPointCloud();
    }
    else if(PointCloudParamterConfig::getFileType() == PointCloudFileType::BIN_FILE)
    {
        if(PointCloudParamterConfig::getFieldsNumber() == 3)
        {
            pcl::PointCloud<pcl::PointXYZI>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZI>);
            errorCode = pcReader.xyzBinRead(pcdFilePath.toStdString(), cloud);
            dataFlag = 1;
            convertRGBPointCloud(cloud);
            drawRGBPointCloud();
        }
        else if(PointCloudParamterConfig::getFieldsNumber() == 4)
        {
            pcl::PointCloud<pcl::PointXYZI>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZI>);
            errorCode = pcReader.xyziBinRead(pcdFilePath.toStdString(), cloud);
            dataFlag = 1;
            convertRGBPointCloud(cloud);
            drawRGBPointCloud();
        }
        else if(PointCloudParamterConfig::getFieldsNumber() == 6)
        {
            errorCode = pcReader.rbgBinRead(pcdFilePath.toStdString(), rgbCloud);
            dataFlag = 1;
            drawRGBPointCloud();
        }
    }
    else if(PointCloudParamterConfig::getFileType() == PointCloudFileType::PLY_FILE)
    {
        if(PointCloudParamterConfig::getIsMesh())
        {
            srcMesh.reset (new pcl::PolygonMesh);
            try{
                errorCode = pcl::io::loadPolygonFilePLY(pcdFilePath.toStdString(), *srcMesh);
                std::cout << "srcMesh:" << errorCode << std::endl;
                drawPolygonMesh();
             }
             catch(std::bad_exception& e)
             {
                errorCode = -1;
             }
        }
        else
        {
            errorCode = pcReader.plyRead(pcdFilePath.toStdString(), srcCloud);
            dataFlag = 0;
            drawTypeColorPointCloud();
        }
    }
    else if(PointCloudParamterConfig::getFileType() == PointCloudFileType::OBJ_FILE)
    {
//        srcMesh.reset (new pcl::PolygonMesh);
//        try{
//            pcl::TextureMesh mesh;
//            std::cout << "1srcMesh:" << pcdFilePath.toStdString() << std::endl;
//            pcl::io::loadOBJFile(pcdFilePath.toStdString(), mesh);
//            std::cout << "2srcMesh:" << pcdFilePath.toStdString() << std::endl;
//            errorCode = pcl::io::loadPolygonFileOBJ(pcdFilePath.toStdString(), *srcMesh);
//            std::cout << "3srcMesh:" << errorCode << std::endl;
//            // drawPolygonMesh();
//         }
//         catch(std::bad_exception& e)
//         {
//            errorCode = -1;
//         }
    }
    return errorCode;
}

void PCLViewer::setOjects(const QList<MyObject> &obejcts, QString sampleClass)
{
    QList<MyObject> rect3DObejcts;
    rect3DObejcts.clear();
    for(int loop = 0; loop < obejcts.count(); loop++)
    {
        const MyObject object = obejcts[loop];
        if(object.getShapeType() == ShapeType::RECT3D_SHAPE)
        {
            rect3DObejcts.append(object);
        }
    }
    this->sampleClass = sampleClass;
    drawShape(rect3DObejcts);
}

void PCLViewer::clearPoints()
{
    viewer->removeAllShapes();
    viewer->removePointCloud("srcCloud");
    if(clickedPoints->size() > 0)
    {
        clickedPoints->clear();
        viewer->updatePointCloud(clickedPoints, "clickedPoints");
        this->update();
    }
}

void PCLViewer::getPoints(pcl::PointCloud<pcl::PointXYZI>::Ptr &result)
{
    result->clear();
    for(const pcl::PointXYZRGB &point: clickedPoints->points)
    {
        pcl::PointXYZI tempPoint;
        tempPoint.x = point.x;
        tempPoint.y = point.y;
        tempPoint.z = point.z;
        tempPoint.intensity = 0;
        result->push_back(tempPoint);
    }
}

void PCLViewer::enterEvent(QEvent *e)
{
    if(isSelect)
    {
        setCursor(myDrawCursor);
    }
    QVTKWidget::enterEvent(e);
}

void PCLViewer::leaveEvent(QEvent *e)
{
    QVTKWidget::leaveEvent(e);
}

void PCLViewer::mouseDoubleClickEvent(QMouseEvent *e)
{
    QVTKWidget::mouseDoubleClickEvent(e);
}

void PCLViewer::clickedPointCallback(const pcl::visualization::PointPickingEvent& event)
{
    if(isSelect)
    {
        bool isIn = false;
        pcl::PointXYZRGB currentPoint;
        if (event.getPointIndex() == -1)
            return;
        event.getPoint(currentPoint.x, currentPoint.y, currentPoint.z);
        currentPoint.r = 0;
        currentPoint.g = 0;
        currentPoint.b = 0;
        for(size_t loop = 0; loop < this->clickedPoints->size(); loop++)
        {
            if(this->clickedPoints->points[loop].x != currentPoint.x
                    && this->clickedPoints->points[loop].y != currentPoint.y
                    && this->clickedPoints->points[loop].z != currentPoint.z)
            {
                isIn = true;
                break;
            }
        }
        if(isIn || this->clickedPoints->size() <= 0)
            this->clickedPoints->push_back(currentPoint);

        pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> red(clickedPoints, 255, 0, 0);

        this->viewer->removePointCloud("clickedPoints");
        this->viewer->addPointCloud(this->clickedPoints, red, "clickedPoints");
        this->viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "clickedPoints");

        this->update();

        std::cout << "point:" << currentPoint.x << " " << currentPoint.y << " " << currentPoint.z << std::endl;
    }
}

void PCLViewer::clickedAreaCallback(const pcl::visualization::AreaPickingEvent& event)
{
  if(isSelect)
  {
      std::vector< int > indices;
      if(event.getPointsIndices(indices)==-1)
        return;

      pcl::PointCloud<pcl::PointXYZ> cloud;
      pcl::fromPCLPointCloud2(*srcCloud, cloud);

      for (int i = 0; i < indices.size(); ++i)
      {
        bool isIn = false;
        pcl::PointXYZRGB currentPoint;
        if(dataFlag == 0)
        {
            currentPoint.x = cloud.points.at(indices[i]).x;
            currentPoint.y = cloud.points.at(indices[i]).y;
            currentPoint.z = cloud.points.at(indices[i]).z;
            currentPoint.r = 0;
            currentPoint.g = 0;
            currentPoint.b = 0;
        }
        else if(dataFlag == 1)
        {
            currentPoint = rgbCloud->points.at(indices[i]);
        }
        else
        {
            return;
        }

        for(size_t loop = 0; loop < this->clickedPoints->size(); loop++)
        {
            if(this->clickedPoints->points[loop].x != currentPoint.x
                    && this->clickedPoints->points[loop].y != currentPoint.y
                    && this->clickedPoints->points[loop].z != currentPoint.z)
            {
                isIn = true;
                break;
            }
        }
        if(isIn || this->clickedPoints->size() <= 0)
            this->clickedPoints->push_back(currentPoint);
      }

      pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZRGB> red(clickedPoints, 255, 0, 0);

      viewer->removePointCloud("clickedPoints");
      viewer->addPointCloud(clickedPoints, red, "clickedPoints");
      viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "clickedPoints");

      this->update();

      for (int i = 0; i < clickedPoints->points.size(); i++)
        std::cout << clickedPoints->points[i].x << ", " << clickedPoints->points[i].y << ", " << clickedPoints->points[i].z << ", " << std::endl;
      std::cout << "new size = " << clickedPoints->points.size() << std::endl;
  }
}

void PCLViewer::drawPolygonMesh()
{
    viewer->removePolygonMesh("srcMesh");
    viewer->addPolygonMesh(*srcMesh, "srcMesh");

    this->update();
}

void PCLViewer::drawTypeColorPointCloud()
{
    viewer->removePointCloud("srcCloud");
    geometryHandler.reset(new pcl::visualization::PointCloudGeometryHandlerXYZ<pcl::PCLPointCloud2>(srcCloud));
    if(PointCloudParamterConfig::getColorRenderType() == "intensity" && \
       PointCloudParamterConfig::getFieldsNumber() >= 4)
    {
        colorHandler.reset(new pcl::visualization::PointCloudColorHandlerGenericField<pcl::PCLPointCloud2>(srcCloud,
                                                                                                           "intensity"));
    }
    else if(PointCloudParamterConfig::getColorRenderType() == "x" ||
            PointCloudParamterConfig::getColorRenderType() == "y" ||
            PointCloudParamterConfig::getColorRenderType() == "z")
    {
        colorHandler.reset(new pcl::visualization::PointCloudColorHandlerGenericField<pcl::PCLPointCloud2>(srcCloud,
                                                                                                           PointCloudParamterConfig::getColorRenderType().toStdString()));
    }
    else if(PointCloudParamterConfig::getColorRenderType() == "rgb" && \
             PointCloudParamterConfig::getFieldsNumber() == 6)
    {
        colorHandler.reset (new pcl::visualization::PointCloudColorHandlerRGBField<pcl::PCLPointCloud2>(srcCloud));
    }
    else
    {
        colorHandler.reset(new pcl::visualization::PointCloudColorHandlerRandom<pcl::PCLPointCloud2>(srcCloud));
    }

    viewer->addPointCloud(srcCloud, geometryHandler, colorHandler, \
                          origin, orientation, "srcCloud", 0);
    viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, \
                                             PointCloudParamterConfig::getPointSize(), "srcCloud", 0);

//    viewer->removeAllShapes();
//    viewer->addCube(0, 1.0, 0, 1.0, 0, 1.0, 1, 0, 0, "cube1", 0);
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, \
//                                        pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "cube1");
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 255, 255, 0, "cube1");
//    viewer->addCube(0, 3.0, 0, 3.0, 0, 3.0, 1, 0, 0, "cube2", 0);
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, \
//                                        pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "cube2");
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 255, 255, 0, "cube2");

    this->update();
}

void PCLViewer::drawRGBPointCloud()
{
    viewer->removePointCloud("srcCloud");
    if(PointCloudParamterConfig::getColorRenderType() == "x" ||
            PointCloudParamterConfig::getColorRenderType() == "y" ||
            PointCloudParamterConfig::getColorRenderType() == "z")
    {
        pcl::visualization::PointCloudColorHandlerGenericField<pcl::PointXYZRGB> fildColor(rgbCloud,
                                                                                           PointCloudParamterConfig::getColorRenderType().toStdString());
        viewer->addPointCloud(rgbCloud, fildColor, "srcCloud");
    }
    else
    {
        viewer->addPointCloud(rgbCloud, "srcCloud");
    }
    viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, \
                                             PointCloudParamterConfig::getPointSize(), "srcCloud", 0);

//    viewer->removeAllShapes();
//    viewer->addCube(0, 1.0, 0, 1.0, 0, 1.0, 1, 0, 0, "cube1", 0);
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, \
//                                        pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "cube1");
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 255, 255, 0, "cube1");
//    viewer->addCube(0, 3.0, 0, 3.0, 0, 3.0, 1, 0, 0, "cube2", 0);
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, \
//                                        pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "cube2");
//    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 255, 255, 0, "cube2");

    this->update();
}

void PCLViewer::drawShape(const QList<MyObject> &obejcts)
{
    viewer->removeAllShapes();
    drawCircle();
    for(int loop = 0; loop < obejcts.count(); loop++)
    {
        if(sampleClass == "All")
        {
            drawObject(obejcts[loop], loop);
        }
        else
        {
            if(obejcts[loop].getObjectClass().contains(sampleClass))
            {
                drawObject(obejcts[loop], loop);
            }
        }
    }
    this->update();
}

void PCLViewer::drawCircle()
{
    pcl::ModelCoefficients circleCoeff1;
    circleCoeff1.values.resize(3);
    circleCoeff1.values[0] = 0;
    circleCoeff1.values[1] = 0;
    circleCoeff1.values[2] = 50.0f;
    viewer->addCircle(circleCoeff1, "circle1", 0);
    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, \
                                        pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "circle1");
    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 255, 0, 0, "circle1");

    pcl::ModelCoefficients circleCoeff2;
    circleCoeff2.values.resize(3);
    circleCoeff2.values[0] = 0;
    circleCoeff2.values[1] = 0;
    circleCoeff2.values[2] = 100.0f;
    viewer->addCircle(circleCoeff2, "circle2", 0);
    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, \
                                        pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, "circle2");
    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 255, 255, 0, "circle2");
}

void PCLViewer::drawObject(const MyObject &object, int id)
{
    pcl::PointXYZ point;
    const MyRect3D rect3d = object.getBox3D();
    QString boxId = QString("box_%1").arg(id);
    Rotation yaw = transform.getRotation(0, 0, rect3d.theta);
    double length = static_cast<double>(rect3d.size[0]);
    double width = static_cast<double>(rect3d.size[1]);
    double height = static_cast<double>(rect3d.size[2]);
    point.x = rect3d.center[0];
    point.y = rect3d.center[1];
    point.z = rect3d.center[2] + static_cast<float>(height / 2.0);
    viewer->addText3D<pcl::PointXYZ>(object.getObjectClass().toStdString(), point, 1.0, \
                                     255, 255, 0, QString("text_%1").arg(id).toStdString());
    viewer->addCube(rect3d.center, yaw, length, width, height, boxId.toStdString());
    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_REPRESENTATION, \
                                        pcl::visualization::PCL_VISUALIZER_REPRESENTATION_WIREFRAME, boxId.toStdString());
    viewer->setShapeRenderingProperties(pcl::visualization::PCL_VISUALIZER_COLOR, 255, 0, 0, boxId.toStdString());
}

void PCLViewer::convertRGBPointCloud(const pcl::PointCloud<pcl::PointXYZI>::Ptr &cloud)
{
    float rScale = 1;
    float gScale = 0.5;
    float bScale = 0.1;
    rgbCloud->clear();
    // Fill the cloud with some points
    for (size_t i = 0; i < cloud->points.size (); ++i)
    {
      pcl::PointXYZRGB point;
      point.x = cloud->points[i].x;
      point.y = cloud->points[i].y;
      point.z = cloud->points[i].z;
      point.r = static_cast<unsigned char>(cloud->points[i].intensity * rScale);
      point.g = static_cast<unsigned char>(cloud->points[i].intensity * gScale);
      point.b = static_cast<unsigned char>(cloud->points[i].intensity * bScale);
      rgbCloud->push_back(point);
    }
}

void PCLViewer::initData()
{
    viewer.reset(new pcl::visualization::PCLVisualizer("cloud", false));
    initCloud();

    colorHandler.reset(new pcl::visualization::PointCloudColorHandlerRandom<pcl::PCLPointCloud2>(srcCloud));
    geometryHandler.reset(new pcl::visualization::PointCloudGeometryHandlerXYZ<pcl::PCLPointCloud2>(srcCloud));
    //geometryHandler.reset(new pcl::visualization::PointCloudGeometryHandlerSurfaceNormal<pcl::PCLPointCloud2>(srcCloud));
    viewer->addPointCloud(srcCloud, geometryHandler, colorHandler, origin, orientation, "srcCloud", 0);
    viewer->setPointCloudRenderingProperties(pcl::visualization::PCL_VISUALIZER_POINT_SIZE, \
                                             PointCloudParamterConfig::getPointSize(), "srcCloud", 0);
    viewer->setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_IMMEDIATE_RENDERING, 1.0, "srcCloud");

    this->SetRenderWindow(viewer->getRenderWindow());
    viewer->setupInteractor(this->GetInteractor (), this->GetRenderWindow ());

    viewer->setBackgroundColor(0, 0, 0);
    viewer->setCameraPosition(0, 0, 0, 0, 0, 0, 0, 0, -1);
    viewer->addCoordinateSystem(1.0);
    viewer->initCameraParameters();
    viewer->resetCamera();

    viewer->registerPointPickingCallback(boost::bind(&PCLViewer::clickedPointCallback, this, _1));
    viewer->registerAreaPickingCallback(boost::bind(&PCLViewer::clickedAreaCallback, this, _1));

    // Set whether or not we should be using the vtkVertexBufferObjectMapper
    viewer->setUseVbos(true);

    this->update();

    sampleClass = "All";

    isSelect = false;
    myDrawCursor = QCursor(QPixmap(tr(":/images/images/cross.png")));
}

void PCLViewer::initConnect()
{

}

void PCLViewer::initCloud()
{
    dataFlag = -1;
    srcMesh.reset(new pcl::PolygonMesh);
    srcCloud.reset(new pcl::PCLPointCloud2);
    rgbCloud.reset(new pcl::PointCloud<pcl::PointXYZRGB>);
    clickedPoints.reset(new pcl::PointCloud<pcl::PointXYZRGB>);
}
