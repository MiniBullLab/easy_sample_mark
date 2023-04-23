#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "birdviewprocess.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <cmath>
#include <sstream>

#include <Eigen/Dense>

BirdViewProcess::BirdViewProcess(QDialog *parent) : QDialog(parent)
{
    init();
    initUI();
    initConnect();
}

BirdViewProcess::~BirdViewProcess()
{

}

void BirdViewProcess::slotOpenImage()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    openDataDir,
                                                    tr("Images (*.png *.jpg *.jpeg)"));
    if(fileName.trimmed().isEmpty())
        return;
    if(currentImage.load(fileName))
    {
        QFileInfo imageFileInfo(fileName);
        openDataDir = imageFileInfo.path();
        QString paramPath = openDataDir + "/homography.json";
        imageShow->setNewQImage(currentImage);
        if(loadHomographyParam(paramPath))
        {
            std::ostringstream tempStr;
            tempStr << "Homography:\n" << homography << "\n";
            this->commandText->append(QString::fromStdString(tempStr.str()));
            imageShow->setPointList(pointList);
            for(size_t i = 0; i < birdPointList.size(); i++)
            {
                int index = birdPointTable->rowCount();
                birdPointTable->insertRow(index);
                for (int col = 0; col < 3; ++col)
                {
                    QTableWidgetItem *item = new QTableWidgetItem("");
                    birdPointTable->setItem(index, col, item);
                    birdPointTable->setCurrentItem(item);
                }
                birdPointTable->setItemDelegateForRow(index, new EditNumberItemDelegate(this));

                birdPointTable->item(index, 0)->setData(Qt::EditRole, QVariant(birdPointList[i].x));
                birdPointTable->item(index, 1)->setData(Qt::EditRole, QVariant(birdPointList[i].y));
                birdPointTable->item(index, 2)->setData(Qt::EditRole, QVariant(0));
            }

            selectPointButton->setEnabled(true);
            birdCalibrationButton->setEnabled(true);
            birdViewButton->setEnabled(true);
            saveResultButton->setEnabled(true);
            distanceButton->setEnabled(true);
        }
        else
        {
            selectPointButton->setEnabled(true);
            birdCalibrationButton->setEnabled(false);
            birdViewButton->setEnabled(false);
            saveResultButton->setEnabled(false);
            distanceButton->setEnabled(false);
        }
    }
    else
    {
         QMessageBox::information(this, tr("加载图片"), tr("该图片加载失败！"));
    }
}

void BirdViewProcess::slotSelectPoint()
{
    if(selectPointButton->text() == QString(tr("开始选择点")))
    {
        imageShow->setNewQImage(currentImage);
        while(birdPointTable->rowCount() > 0)
        {
            birdPointTable->removeRow(0);
        }
        pointList.clear();
        birdPointList.clear();
        imageShow->setEnabled(true);
        selectPointButton->setText(tr("停止选择点"));
        selectPointButton->setEnabled(true);
        birdCalibrationButton->setEnabled(false);
        birdViewButton->setEnabled(false);
        saveResultButton->setEnabled(false);
        distanceButton->setEnabled(false);
    }
    else if(selectPointButton->text() == QString(tr("停止选择点")))
    {
        if(pointList.count() >= 4)
        {
            for(int row = 0; row < birdPointTable->rowCount(); row++)
            {
                cv::Point2f point(birdPointTable->item(row, 0)->data(Qt::EditRole).toDouble(),
                                   birdPointTable->item(row, 1)->data(Qt::EditRole).toDouble());
                birdPointList.push_back(point);
            }
            selectPointButton->setEnabled(true);
            birdCalibrationButton->setEnabled(true);
            birdViewButton->setEnabled(false);
            saveResultButton->setEnabled(false);
            distanceButton->setEnabled(false);
        }
        else
        {
            QMessageBox::information(this, tr("选择点"), tr("选择点数必须大于等于4, 请重新选择点！"));
        }
        imageShow->setEnabled(false);
        selectPointButton->setText(tr("开始选择点"));
    }
}

void BirdViewProcess::slotBirdCalibration()
{
    std::vector<cv::Point2f> dst_pts;
    std::vector<cv::Point2f> src_pts;
    if(pointList.count() >= 4)
    {
        for(int i = 0; i < pointList.count(); i++)
        {
            cv::Point2f src_pt(pointList[i].x(), pointList[i].y());
            dst_pts.push_back(cv::Point2f(birdPointList[i].x, birdPointList[i].y));
            src_pts.push_back(src_pt);
        }

        homography = cv::findHomography(src_pts, dst_pts);
        std::ostringstream tempStr;
        tempStr << "Homography:\n" << homography << "\n";
        commandText->append(QString::fromStdString(tempStr.str()));

        selectPointButton->setEnabled(true);
        birdCalibrationButton->setEnabled(true);
        birdViewButton->setEnabled(true);
        saveResultButton->setEnabled(true);
        distanceButton->setEnabled(true);
    }
    else
    {
        QMessageBox::information(this, tr("选择点"), tr("选择点数必须大于等于4！"));
    }
}

void BirdViewProcess::slotBirdView()
{
    QImage image = currentImage.convertToFormat(QImage::Format_RGB888);
    cv::Mat imageInput = convertImage.QImageTocvMat(image).clone();
    cv::Mat birdImage;
    cv::Mat rgbFrame;
//    cv::Mat h = cv::Mat(3, 3, CV_32F, cv::Scalar::all (0));
//    cv::Rect boundingRect;
//    std::vector<cv::Point2f> dst_pts;
//    std::vector<cv::Point2f> src_pts;
//    for(int i = 0; i < pointList.count(); i++)
//    {
//        cv::Point2f src_pt(pointList[i].x(), pointList[i].y());
//        src_pts.push_back(src_pt);
//    }
//    boundingRect = cv::boundingRect(src_pts);
//    int width = boundingRect.width;
//    int height = boundingRect.height;
//    cv::Point center;
//    cv::Point2f point1(center.x, center.y);
//    cv::Point2f point2(center.x, center.y);
//    cv::Point2f point3(center.x, center.y);
//    cv::Point2f point4(center.x, center.y);
//    center.x = boundingRect.tl().x + width / 2;
//    center.y = boundingRect.tl().y + height / 2;
//    for(int i = 0; i < pointList.count(); i++)
//    {
//        int x = pointList[i].x();
//        int y = pointList[i].y();
//        float temp1 = computeDistance(cv::Point2f(x, y), center);
//        if(x < center.x && y < center.y)
//        {
//            float temp2 = computeDistance(point1, center);
//            if(temp1 > temp2)
//            {
//                point1 = cv::Point2f(x, y);
//            }
//        }
//        else if(x > center.x && y < center.y)
//        {
//            float temp2 = computeDistance(point2, center);
//            if(temp1 > temp2)
//            {
//                point2 = cv::Point2f(x, y);
//            }
//        }
//        else if(x > center.x && y > center.y)
//        {
//            float temp2 = computeDistance(point3, center);
//            if(temp1 > temp2)
//            {
//                point3 = cv::Point2f(x, y);
//            }
//        }
//        else if(x < center.x && y > center.y)
//        {
//            float temp2 = computeDistance(point4, center);
//            if(temp1 > temp2)
//            {
//                point4 = cv::Point2f(x, y);
//            }
//        }
//    }
//    src_pts.clear();
//    dst_pts.clear();
//    src_pts.push_back(point1);
//    src_pts.push_back(point2);
//    src_pts.push_back(point3);
//    src_pts.push_back(point4);
//    dst_pts.push_back(cv::Point(0, 0));
//    dst_pts.push_back(cv::Point(width, 0));
//    dst_pts.push_back(cv::Point(width, 0));
//    dst_pts.push_back(cv::Point(width, height));

//    for(size_t i = 0; i < src_pts.size(); i++)
//    {
//        cv::circle(imageInput, cv::Point(src_pts[i].x, src_pts[i].y), 3, cv::Scalar(0, 0, 255), -1);
//    }
//    cv::imwrite("image.jpg", imageInput);

//    h = cv::getPerspectiveTransform(dst_pts, src_pts);

    cv::warpPerspective(imageInput, birdImage, homography, imageInput.size() , \
                        cv::INTER_LINEAR + cv::WARP_INVERSE_MAP + cv::WARP_FILL_OUTLIERS);
    cv::cvtColor(birdImage, rgbFrame, cv::COLOR_BGR2RGB);
    birdViewImage = QImage((uchar*)rgbFrame.data, rgbFrame.cols, rgbFrame.rows, QImage::Format_RGB888);
    imageShow->setNewQImage(birdViewImage);
}

void BirdViewProcess::slotSaveResult()
{
    QString saveJsonPath = openDataDir + "/" + "homography" + ".json";
    QJsonDocument doc;
    QByteArray data;
    QJsonObject jsonData;
    QJsonObject allData;
    QFile file(saveJsonPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |QIODevice::Text))
    {
        return;
    }
    jsonData.insert("point_count", pointList.count());

    QJsonArray objectArrays1;
    int index1 = 0;
    for(int loop = 0; loop < pointList.count(); loop++)
    {
        QJsonObject objectData;
        objectData.insert("x", pointList[loop].x());
        objectData.insert("y", pointList[loop].y());
        objectArrays1.insert(index1, objectData);
        index1++;
    }
    if(objectArrays1.count() > 0)
    {
        allData.insert("src_quad", objectArrays1);
    }

    QJsonArray objectArrays2;
    int index2 = 0;
    for(size_t loop = 0; loop < birdPointList.size(); loop++)
    {
        QJsonObject objectData;
        objectData.insert("x", birdPointList[loop].x);
        objectData.insert("y", birdPointList[loop].y);
        objectArrays2.insert(index2, objectData);
        index2++;
    }
    if(objectArrays2.count() > 0)
    {
        allData.insert("dst_quad", objectArrays2);
    }

    jsonData.insert("camera_homography_point", allData);

    QJsonArray homographyData;
    int tempIndex = 0;
    for(int r = 0; r < homography.rows; r++)
    {
        for(int c = 0; c < homography.cols; c++)
        {
            homographyData.insert(tempIndex++, homography.at<float>(r, c));
        }
    }

    jsonData.insert("camera_homography", homographyData);

    doc.setObject(jsonData);
    data = doc.toJson();
    file.write(data);
    file.close();

    commandText->append(QString("Save file:") + saveJsonPath);
}

void BirdViewProcess::slotComputeDistance()
{

}

void BirdViewProcess::slotAddBirdPoint(QPoint point)
{
    int index = birdPointTable->rowCount();
    birdPointTable->insertRow(index);
    for (int col = 0; col < 3; ++col)
    {
        QTableWidgetItem *item = new QTableWidgetItem("");
        birdPointTable->setItem(index, col, item);
        birdPointTable->setCurrentItem(item);
    }

    birdPointTable->setItemDelegateForRow(index, new EditNumberItemDelegate(this));
    pointList.append(point);
}

void BirdViewProcess::init()
{
    openDataDir = ".";
    this->currentImage = QImage(tr(":/images/images/play.png"));
    this->birdViewImage = QImage(tr(":/images/images/play.png"));
    pointList.clear();
    birdPointList.clear();
    homography = cv::Mat(3, 3, CV_32F, cv::Scalar::all (0));
}

void BirdViewProcess::initUI()
{
    imageShow = new ImageLabel();
    imageShow->setNewQImage(currentImage);
    imageShow->setEnabled(false);
    scrollArea = new QScrollArea();
    scrollArea->setAlignment(Qt::AlignCenter);
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setAutoFillBackground(true);
    //scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);  //控件大小 小于 视窗大小时，默认不会显示滚动条
    //scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);    //强制显示滚动条。
    scrollArea->setWidget(imageShow);

    openImageButton = new QPushButton(tr("打开图片"));
    selectPointButton = new QPushButton(tr("开始选择点"));
    selectPointButton->setEnabled(false);
    birdCalibrationButton = new QPushButton(tr("获取单应矩阵"));
    birdCalibrationButton->setEnabled(false);
    birdViewButton = new QPushButton(tr("生成鸟瞰图"));
    birdViewButton->setEnabled(false);
    saveResultButton = new QPushButton(tr("保存结果"));
    saveResultButton->setEnabled(false);
    distanceButton = new QPushButton(tr("计算离相机的距离"));
    distanceButton->setEnabled(false);

    birdPointTable = new QTableWidget();
    birdPointTable->setMouseTracking(true);
    birdPointTable->clear();
    QStringList headerName;
    headerName << QString(tr("X")) << QString(tr("Y")) << QString(tr("Z"));
    birdPointTable->setRowCount(0);
    birdPointTable->setColumnCount(3);
    birdPointTable->setHorizontalHeaderLabels(headerName);
    birdPointTable->verticalHeader()->setVisible(false);
    birdPointTable->horizontalHeader()->setStretchLastSection(true);
    // birdPointTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    birdPointTable->horizontalHeader()->setFixedHeight(25); //设置表头的高度
    birdPointTable->setSelectionBehavior(QTableWidget::SelectRows);
    birdPointTable->setFrameShape(QFrame::NoFrame); //设置无边框
    birdPointTable->setSelectionMode(QAbstractItemView::SingleSelection);

    birdPointTable->horizontalHeader()->resizeSection(0, 80); //设置表头第一列的宽度
    birdPointTable->horizontalHeader()->resizeSection(1, 80); //设置表头第一列的宽度
    birdPointTable->horizontalHeader()->resizeSection(2, 80); //设置表头第一列的宽度

    ratioLabel = new QLabel(tr("1m代表:"));
    ratioBox = new QSpinBox();
    ratioBox->setValue(1);
    ratioBox->setMinimum(1);
    ratioBox->setSingleStep(10);
    ratioBox->setSuffix("pixel");

    QHBoxLayout *tempRightLayout1 = new QHBoxLayout();
    tempRightLayout1->setSpacing(20);
    tempRightLayout1->addWidget(ratioLabel);
    tempRightLayout1->addWidget(ratioBox);

    QVBoxLayout *paramLayout = new QVBoxLayout();
    // paramLayout->setSpacing(20);
    paramLayout->addLayout(tempRightLayout1);
    paramGroundBox = new QGroupBox(tr("参数"));
    paramGroundBox->setLayout(paramLayout);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);
    rightLayout->addWidget(openImageButton);
    rightLayout->addWidget(selectPointButton);
    rightLayout->addWidget(birdCalibrationButton);
    rightLayout->addWidget(birdViewButton);
    rightLayout->addWidget(saveResultButton);
    rightLayout->addWidget(distanceButton);
    rightLayout->addWidget(birdPointTable);
    rightLayout->addWidget(paramGroundBox);

    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(10);
    centerLayout->addWidget(scrollArea);
    centerLayout->addLayout(rightLayout);
    centerLayout->setStretchFactor(scrollArea, 4);
    centerLayout->setStretchFactor(rightLayout, 1);

    commandText = new MyTextBrowser();
    //commandText->setFixedHeight(100);
    commandText->setReadOnly(true);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(10); //设置这个对话框的边距
    mainLayout->setSpacing(10);  //设置各个控件之间的边距
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(centerLayout);
    mainLayout->addWidget(commandText);
    mainLayout->setStretchFactor(centerLayout, 5);
    mainLayout->setStretchFactor(commandText, 1);
    this->setLayout(mainLayout);
    //this->setMaximumSize(700,520);
    this->setMinimumSize(1250, 800);
    this->setWindowTitle(tr("birdview标定"));
}

void BirdViewProcess::initConnect()
{
    connect(openImageButton, &QPushButton::clicked, this, &BirdViewProcess::slotOpenImage);
    connect(selectPointButton, &QPushButton::clicked, this, &BirdViewProcess::slotSelectPoint);
    connect(birdCalibrationButton, &QPushButton::clicked, this, &BirdViewProcess::slotBirdCalibration);
    connect(birdViewButton, &QPushButton::clicked, this, &BirdViewProcess::slotBirdView);
    connect(saveResultButton, &QPushButton::clicked, this, &BirdViewProcess::slotSaveResult);
    connect(distanceButton, &QPushButton::clicked, this, &BirdViewProcess::slotComputeDistance);

    connect(imageShow, &ImageLabel::signalSelectPoint, this, &BirdViewProcess::slotAddBirdPoint);
}

bool BirdViewProcess::loadHomographyParam(const QString &filePath)
{
    bool result = false;
    QByteArray data;
    QFile file;
    file.setFileName(filePath);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        return result;
    }
    data = file.readAll();
    file.close();
    QJsonParseError jsonError;
    QJsonDocument parseDoucment = QJsonDocument::fromJson(QString(data).toUtf8(), &jsonError);

    if(jsonError.error == QJsonParseError::NoError)
    {
        if (!(parseDoucment.isNull() || parseDoucment.isEmpty()))
        {
            if (parseDoucment.isObject())
            {
                QJsonObject allData = parseDoucment.object();
                pointList.clear();
                birdPointList.clear();
                if(allData.contains("src_quad"))
                {
                    QJsonArray dataList = allData.take("src_quad").toArray();
                    for(int index = 0; index < dataList.size(); index++)
                    {
                        QJsonObject tempData = dataList.at(index).toObject();
                        int x = tempData.take("x").toVariant().toInt();
                        int y = tempData.take("y").toVariant().toInt();
                        pointList.append(QPoint(x, y));
                    }
                }
                if(allData.contains("dst_quad"))
                {
                    QJsonArray dataList = allData.take("dst_quad").toArray();
                    for(int index = 0; index < dataList.size(); index++)
                    {
                        QJsonObject tempData = dataList.at(index).toObject();
                        float x = tempData.take("x").toVariant().toFloat();
                        float y = tempData.take("y").toVariant().toFloat();
                        birdPointList.push_back(cv::Point2f(x, y));
                    }
                }
                if(allData.contains("camera_homography"))
                {
                    QJsonArray paramObject = allData.take("camera_homography").toArray();
                    int tempIndex = 0;
                    for(int r = 0; r < homography.rows; r++)
                    {
                        for(int c = 0; c < homography.cols; c++)
                        {
                            homography.at<float>(r, c) = static_cast<float>(paramObject.at(tempIndex++).toDouble());
                        }
                    }
                    result = true;
                }
            }
        }
    }
    else
    {
        qDebug() << "error:" << jsonError.errorString() << endl;
    }
    if(static_cast<int>(birdPointList.size()) == pointList.size())
    {
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}

float BirdViewProcess::computeDistance(const cv::Point2f &point1, const cv::Point2f &point2)
{
    float distance;
    distance = powf((point1.x - point2.x), 2) + powf((point1.y - point2.y), 2);
    distance = sqrtf(distance);
    return distance;
}
