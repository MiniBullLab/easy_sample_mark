#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "pcdconverterwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

PCDConverterWindow::PCDConverterWindow(QWidget *parent) : QWidget(parent)
{
    init();
    initUI();
    initConnect();
}

PCDConverterWindow::~PCDConverterWindow()
{
    if (pcdConverterThread != NULL)
    {
        pcdConverterThread->stopThread();
        pcdConverterThread->wait();
        delete pcdConverterThread;
        pcdConverterThread = NULL;
    }
}

void PCDConverterWindow::slotOpen()
{
    pathDir = QFileDialog::getExistingDirectory(this, tr("选择文件夹"),tr("."),QFileDialog::ShowDirsOnly);
    if(pathDir.trimmed().isEmpty())
    {
        qDebug() << "打开的文件路径有误:" << pathDir << endl;
        return;
    }
    this->pathText->setText(pathDir);
    startButton->setEnabled(true);
}

void PCDConverterWindow::slotStart()
{
    QString suffix = srcFormatBox->currentText();
    QString format = dstFormatBox->currentText();
    int fieldsNumber = fieldsNumberBox->value();
    if(pcdConverterThread->initData(pathDir, suffix, format, fieldsNumber) == 0)
    {
        pcdConverterThread->startThread();
        pcdConverterThread->start();
        stopButton->setEnabled(true);
        startButton->setEnabled(false);
        openButton->setEnabled(false);
    }
    else
    {
        QMessageBox::information(this, tr("点云格式转换"), tr("input error!"));
    }
}

void PCDConverterWindow::slotStop()
{
    pcdConverterThread->stopThread();
    stopButton->setEnabled(false);
    startButton->setEnabled(true);
    openButton->setEnabled(true);
}

void PCDConverterWindow::slotFinish(QString name)
{
    qDebug() << "save path:" << name;
    stopButton->setEnabled(false);
    startButton->setEnabled(true);
    openButton->setEnabled(true);
}

void PCDConverterWindow::closeEvent(QCloseEvent *event)
{
    if(pcdConverterThread->isRunning())
    {
        QMessageBox::StandardButton button;
        button=QMessageBox::question(this,tr("点云格式转换"),QString(tr("点云格式转换程序正在运行，是否退出？")),
                                     QMessageBox::Yes|QMessageBox::No);
        if(button==QMessageBox::No)
        {
            event->ignore();
        }
        else if(button==QMessageBox::Yes)
        {
            pcdConverterThread->stopThread();
            pcdConverterThread->wait();
            event->accept();
            QWidget::closeEvent(event);
            emit signalClosePCDConverterWindow("pcdConverter");
        }
    }
    else
    {
        event->accept();
        QWidget::closeEvent(event);
        emit signalClosePCDConverterWindow("pcdConverter");
    }
}

void PCDConverterWindow::init()
{
    pathDir = "";
    pcdConverterThread = new PCDConverterThread();
}

void PCDConverterWindow::initUI()
{
    mainLayout = new QVBoxLayout();

    QHBoxLayout* layout0 = new QHBoxLayout();

    srcFormatLabel = new QLabel(tr("点云初始格式："));
    srcFormatBox = new QComboBox();
    srcFormatBox->addItem(tr("*.pcd"));
    srcFormatBox->addItem(tr("*.ply"));

    dstFormatLabel = new QLabel(tr("点云转换格式："));
    dstFormatBox = new QComboBox();
    dstFormatBox->addItem(tr(".bin"));

    layout0->addWidget(srcFormatLabel);
    layout0->addWidget(srcFormatBox);
    layout0->addWidget(dstFormatLabel);
    layout0->addWidget(dstFormatBox);

    QHBoxLayout* layout01 = new QHBoxLayout();
    fieldsNumberLabel = new QLabel(tr("转换字段数:"));
    fieldsNumberBox = new QSpinBox();
    fieldsNumberBox->setMinimum(3);
    fieldsNumberBox->setMaximum(6);
    fieldsNumberBox->setSingleStep(1);
    fieldsNumberBox->setValue(3);
    layout01->addWidget(fieldsNumberLabel);
    layout01->addWidget(fieldsNumberBox);

    QHBoxLayout* layout1 = new QHBoxLayout();
    openButton = new QPushButton(tr("打开点云文件夹"));
    pathText = new QLineEdit();
    pathText->setReadOnly(true);
    layout1->addWidget(pathText);
    layout1->addWidget(openButton);

    QHBoxLayout*layout2 = new QHBoxLayout();
    startButton = new QPushButton(tr("开始转换"));
    stopButton = new QPushButton(tr("结束转换"));
    startButton->setEnabled(false);
    stopButton->setEnabled(false);
    layout2->setSpacing(50);
    layout2->addWidget(startButton);
    layout2->addWidget(stopButton);

    mainLayout->addLayout(layout0);
    mainLayout->addLayout(layout01);
    mainLayout->addLayout(layout1);
    mainLayout->addLayout(layout2);
    this->setLayout(mainLayout);
    this->setWindowTitle(tr("点云格式转换"));
}

void PCDConverterWindow::initConnect()
{
    connect(openButton, &QPushButton::clicked, this, &PCDConverterWindow::slotOpen);
    connect(startButton, &QPushButton::clicked, this, &PCDConverterWindow::slotStart);
    connect(stopButton, &QPushButton::clicked, this, &PCDConverterWindow::slotStop);
    connect(pcdConverterThread, &PCDConverterThread::signalFinish, this, &PCDConverterWindow::slotFinish);
}
