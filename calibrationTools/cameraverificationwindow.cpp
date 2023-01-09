#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "cameraverificationwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include "helpers/dirprocess.h"

CameraVerificationWindow::CameraVerificationWindow(QDialog *parent) : QDialog(parent)
{
    init();
    initUI();
    initConnect();
}

CameraVerificationWindow::~CameraVerificationWindow()
{

}

void CameraVerificationWindow::slotOpenImageDir()
{
    DirProcess dirProcess;
    processDataList.clear();
    this->openDataDir = QFileDialog::getExistingDirectory(this, tr("选择文件夹"), openDataDir, QFileDialog::ShowDirsOnly);
    if(this->openDataDir.trimmed().isEmpty() || !QDir(this->openDataDir).exists())
    {
        qDebug() << "打开的文件路径有误:" << this->openDataDir << endl;
    }
    else
    {
        QStringList filter;
        filter << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp";
        processDataList = dirProcess.getDirFileName(this->openDataDir, filter);
        imageListWidget->clear();
        for(int index = 0; index < processDataList.size(); index++)
        {
            QListWidgetItem *item = new QListWidgetItem(QIcon(":/qss_icons/style/rc/checkbox_checked_focus.png"),
                                                        processDataList[index]);
            item->setData(Qt::UserRole, 1);
            imageListWidget->insertItem(index, item);
        }
        verificationButton->setEnabled(false);
    }
}

void CameraVerificationWindow::slotImageItem(QListWidgetItem *item)
{
    QString imagePath = item->text();
    if(currentImage.load(imagePath))
    {
        currentImagePath = imagePath;
        imageShow->setNewQImage(currentImage);
        verificationButton->setEnabled(true);
    }
    else
    {
        currentImagePath = "";
        QMessageBox::information(this, tr("加载图片"), tr("该图片加载失败,请点击下一张图片！"));
    }
}

void CameraVerificationWindow::slotVerification()
{
    double d, d_max;
    if(currentImagePath != "")
    {
        std::cout << "currentImagePath:" << currentImagePath.toStdString() << std::endl;
        distortionMeasurement.measure(currentImagePath.toStdString(), d, d_max);
        this->commandText->append(tr("畸变验证："));
        this->commandText->append(tr("d: %1").arg(d));
        this->commandText->append(tr("d_max: %1").arg(d_max));
    }
}

void CameraVerificationWindow::init()
{
    this->currentImage = QImage(tr(":/images/images/play.png"));
    currentImagePath = "";
    openDataDir = ".";
    processDataList.clear();
}

void CameraVerificationWindow::initUI()
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

    commandText = new MyTextBrowser();
    //commandText->setFixedHeight(100);
    commandText->setReadOnly(true);

    QVBoxLayout *centerLayout = new QVBoxLayout();
    centerLayout->setSpacing(10);
    centerLayout->addWidget(scrollArea);
    centerLayout->addWidget(commandText);
    centerLayout->setStretchFactor(scrollArea, 5);
    centerLayout->setStretchFactor(commandText, 1);

    imageListWidget = new QListWidget();

    openDirButton = new QPushButton(tr("打开图片文件夹"));
    verificationButton = new QPushButton(tr("验证图像"));
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(10);
    buttonLayout->addWidget(openDirButton);
    buttonLayout->addWidget(verificationButton);
    verificationButton->setEnabled(false);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(imageListWidget);
    rightLayout->addLayout(buttonLayout);
    rightLayout->setStretchFactor(imageListWidget, 5);
    rightLayout->setStretchFactor(buttonLayout, 1);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setMargin(10); //设置这个对话框的边距
    mainLayout->setSpacing(10);  //设置各个控件之间的边距
    mainLayout->setAlignment(Qt::AlignCenter);
    mainLayout->addLayout(centerLayout);
    mainLayout->addLayout(rightLayout);
    mainLayout->setStretchFactor(centerLayout, 4);
    mainLayout->setStretchFactor(rightLayout, 1);
    this->setLayout(mainLayout);
    //this->setMaximumSize(700,520);
    this->setMinimumSize(1000, 600);
    this->setWindowTitle(tr("相机畸变验证"));
}

void CameraVerificationWindow::initConnect()
{
    connect(openDirButton, &QPushButton::clicked, this, &CameraVerificationWindow::slotOpenImageDir);
    connect(imageListWidget, &QListWidget::itemClicked, this, &CameraVerificationWindow::slotImageItem);
    connect(verificationButton, &QPushButton::clicked, this, &CameraVerificationWindow::slotVerification);
}
