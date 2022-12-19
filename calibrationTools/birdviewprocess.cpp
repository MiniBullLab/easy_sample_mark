#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "birdviewprocess.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

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
                                                    ".",
                                                    tr("Images (*.png *.jpg)"));
    if(fileName.trimmed().isEmpty())
        return;
    if(currentImage.load(fileName))
    {
        imageShow->setNewQImage(currentImage);
        selectPointButton->setEnabled(true);
        birdCalibrationButton->setEnabled(true);
        birdViewButton->setEnabled(true);
        distanceButton->setEnabled(true);
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
        imageShow->setEnabled(true);
        selectPointButton->setText(tr("停止选择点"));
    }
    else if(selectPointButton->text() == QString(tr("停止选择点")))
    {
        imageShow->setEnabled(false);
        selectPointButton->setText(tr("开始选择点"));
    }
}

void BirdViewProcess::slotBirdCalibration()
{
    QList<QPoint> tempList = imageShow->getPointList();
    std::vector<cv::Point2f> objPts(4);
    std::vector<cv::Point2f> imgPts(4);
    if(tempList.count() >= 4)
    {
        imgPts[0] = cv::Point2f(tempList[0].x(), tempList[0].y());
        imgPts[1] = cv::Point2f(tempList[1].x(), tempList[1].y());
        imgPts[2] = cv::Point2f(tempList[2].x(), tempList[2].y());
        imgPts[3] = cv::Point2f(tempList[3].x(), tempList[3].y());

        objPts[0] = cv::Point2f(0, 0);
        objPts[1] = cv::Point2f((float)boxWidthBox->value(), 0);
        objPts[2] = cv::Point2f((float)boxWidthBox->value(), (float)boxHeightBox->value());
        objPts[3] = cv::Point2f(0, (float)boxHeightBox->value());

        homography = cv::getPerspectiveTransform (objPts, imgPts);
        QString tempStr = "";
        for(int r = 0; r < homography.rows; r++)
        {
            for(int c = 0; c < homography.cols; c++)
            {
                tempStr += QString::number(homography.at<float>(r, c));
                tempStr += ",";
            }
            tempStr += "\n";
        }
        commandText->append(tempStr);
    }
    else
    {
        QMessageBox::information(this, tr("选择点"), tr("选择点数必须大于等于4！"));
    }
}

void BirdViewProcess::slotBirdView()
{
    QImage image = currentImage.convertToFormat(QImage::Format_RGB888);
    cv::Mat imageInput = convertImage.QImageTocvMat(image);
    cv::Mat birdImage;
    cv::Mat rgbFrame;
    cv::warpPerspective(imageInput, birdImage, homography, cv::Size(imageInput.cols, imageInput.rows) , \
                        cv::INTER_LINEAR + cv::WARP_INVERSE_MAP + cv::WARP_FILL_OUTLIERS);
    cv::cvtColor(birdImage, rgbFrame, cv::COLOR_BGR2RGB);
    birdViewImage = QImage((uchar*)rgbFrame.data, rgbFrame.cols, rgbFrame.rows, QImage::Format_RGB888);
    imageShow->setNewQImage(birdViewImage);
}

void BirdViewProcess::slotComputeDistance()
{
    QList<QPoint> tempList = imageShow->getPointList();
    foreach(QPoint var, tempList)
    {

    }
}

void BirdViewProcess::init()
{
    this->currentImage = QImage(tr(":/images/images/play.png"));
    allCorners.clear();
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

    boxWidthLable = new QLabel(tr("box宽度:"));
    boxWidthBox = new QDoubleSpinBox();
    boxWidthBox->setValue(1.0);
    boxWidthBox->setSingleStep(1.0);
    boxWidthBox->setMinimum(1.0);
    boxWidthBox->setSuffix("mm");

    QHBoxLayout *rightLayout1 = new QHBoxLayout();
    rightLayout1->setSpacing(20);
    rightLayout1->addWidget(boxWidthLable);
    rightLayout1->addWidget(boxWidthBox);

    boxHeightLable = new QLabel(tr("box高度:"));
    boxHeightBox = new QDoubleSpinBox();
    boxHeightBox->setValue(1.0);
    boxHeightBox->setSingleStep(1.0);
    boxHeightBox->setMinimum(1.0);
    boxHeightBox->setSuffix("mm");

    QHBoxLayout *rightLayout2 = new QHBoxLayout();
    rightLayout2->setSpacing(20);
    rightLayout2->addWidget(boxHeightLable);
    rightLayout2->addWidget(boxHeightBox);

    ratioLabel = new QLabel(tr("1m代表:"));
    ratioBox = new QSpinBox();
    ratioBox->setValue(1);
    ratioBox->setMinimum(1);
    ratioBox->setSingleStep(10);
    ratioBox->setSuffix("pixel");

    QHBoxLayout *rightLayout3 = new QHBoxLayout();
    rightLayout3->setSpacing(20);
    rightLayout3->addWidget(ratioLabel);
    rightLayout3->addWidget(ratioBox);

    chessboardSizeLabel = new QLabel(tr("棋盘格尺寸:"));
    chessboardSizeBox = new QDoubleSpinBox();
    chessboardSizeBox->setValue(1.0);
    chessboardSizeBox->setSingleStep(1.0);
    chessboardSizeBox->setMinimum(1.0);
    chessboardSizeBox->setSuffix("mm");

    QHBoxLayout *rightLayout4 = new QHBoxLayout();
    rightLayout4->setSpacing(20);
    rightLayout4->addWidget(chessboardSizeLabel);
    rightLayout4->addWidget(chessboardSizeBox);

    chessboardCountLabel = new QLabel(tr("棋盘格角点数:"));
    chessboardWBox = new QSpinBox();
    chessboardWBox->setValue(1);
    chessboardWBox->setMinimum(1);
    chessboardWBox->setSingleStep(10);
    chessboardWBox->setPrefix("w:");
    chessboardHBox = new QSpinBox();
    chessboardHBox->setValue(1);
    chessboardHBox->setMinimum(1);
    chessboardHBox->setSingleStep(10);
    chessboardHBox->setPrefix("h:");

    QHBoxLayout *rightLayout5 = new QHBoxLayout();
    rightLayout5->setSpacing(20);
    rightLayout5->addWidget(chessboardCountLabel);
    rightLayout5->addWidget(chessboardWBox);
    rightLayout5->addWidget(chessboardHBox);

    QVBoxLayout *paramLayout = new QVBoxLayout();
    // paramLayout->setSpacing(20);
    paramLayout->addLayout(rightLayout1);
    paramLayout->addLayout(rightLayout2);
    paramLayout->addLayout(rightLayout3);
    paramLayout->addLayout(rightLayout4);
    paramLayout->addLayout(rightLayout5);
    paramGroundBox = new QGroupBox(tr("参数"));
    paramGroundBox->setLayout(paramLayout);

    selectPointButton = new QPushButton(tr("开始选择点"));
    selectPointButton->setEnabled(false);
    birdCalibrationButton = new QPushButton(tr("获取单应矩阵"));
    birdCalibrationButton->setEnabled(false);
    birdViewButton = new QPushButton(tr("生成鸟瞰图"));
    birdViewButton->setEnabled(false);
    distanceButton = new QPushButton(tr("计算离相机的距离"));
    distanceButton->setEnabled(false);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(10);
    rightLayout->addWidget(openImageButton);
    rightLayout->addWidget(paramGroundBox);
    rightLayout->addWidget(selectPointButton);
    rightLayout->addWidget(birdCalibrationButton);
    rightLayout->addWidget(birdViewButton);
    rightLayout->addWidget(distanceButton);

    QHBoxLayout *centerLayout = new QHBoxLayout();
    centerLayout->setSpacing(10);
    centerLayout->addWidget(scrollArea);
    centerLayout->addLayout(rightLayout);

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
    this->setMinimumSize(1000, 600);
    this->setWindowTitle(tr("birdview标定"));
}

void BirdViewProcess::initConnect()
{
    connect(openImageButton, &QPushButton::clicked, this, &BirdViewProcess::slotOpenImage);
    connect(selectPointButton, &QPushButton::clicked, this, &BirdViewProcess::slotSelectPoint);
    connect(birdCalibrationButton, &QPushButton::clicked, this, &BirdViewProcess::slotBirdCalibration);
    connect(birdViewButton, &QPushButton::clicked, this, &BirdViewProcess::slotBirdView);
    connect(distanceButton, &QPushButton::clicked, this, &BirdViewProcess::slotComputeDistance);
}
