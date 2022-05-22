#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "pointcloudmarkparamterwindow.h"
#include <QMessageBox>
#include <limits>

PointCloudMarkParamterWindow::PointCloudMarkParamterWindow(QDialog *parent) : QDialog(parent), paramterConfig()
{
    init();
    initUI();
    initConnect();
}

PointCloudMarkParamterWindow::~PointCloudMarkParamterWindow()
{

}

void PointCloudMarkParamterWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("保存点云标注参数配置信息"), tr("是否保存配置信息?"),
                                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(result == QMessageBox::Yes)
    {
        slotOk();
    }
    QDialog::closeEvent(event);
}

void PointCloudMarkParamterWindow::slotOk()
{
    paramterConfig.setPointSize(this->pointSizeBox->value());
    paramterConfig.setFieldsNumber(this->fieldsNumberBox->value());
    paramterConfig.setColorRenderType(this->colorTypeBox->currentText());
    paramterConfig.setIsMesh(this->isShowMeshBox->isChecked());
    paramterConfig.setFileType(this->formatBox->currentData().toInt());
    paramterConfig.saveConfig();
    this->accept();
}

void PointCloudMarkParamterWindow::loadDefaultValue()
{
    this->pointSizeBox->setValue(2);
    this->fieldsNumberBox->setValue(3);
    this->formatBox->setCurrentIndex(0);
}

void PointCloudMarkParamterWindow::init()
{
    paramterConfig.loadConfig();
}

void PointCloudMarkParamterWindow::initUI()
{
    QHBoxLayout *layout0 = new QHBoxLayout();
    layout0->setSpacing(30);
    formatLabel = new QLabel(tr("读取文件格式:"));
    formatBox = new QComboBox();
    initFileType();
    layout0->addWidget(formatLabel);
    layout0->addWidget(formatBox);

    fieldsNumberLabel = new QLabel(tr("读取字段数:"));
    fieldsNumberBox = new QSpinBox();
    fieldsNumberBox->setMinimum(3);
    fieldsNumberBox->setMaximum(6);
    fieldsNumberBox->setValue(paramterConfig.getFieldsNumber());
    fieldsNumberBox->setSingleStep(1);

    QHBoxLayout *layout1 = new QHBoxLayout();
    layout1->setSpacing(30);
    layout1->addWidget(fieldsNumberLabel);
    layout1->addWidget(fieldsNumberBox);

    isShowMeshBox = new QCheckBox(tr("是否显示Mesh"));
    isShowMeshBox->setChecked(paramterConfig.getIsMesh());

    colorTypeLabel = new QLabel(tr("点云渲染颜色方式:"));
    colorTypeBox = new QComboBox();
    colorTypeBox->addItem("x");
    colorTypeBox->addItem("y");
    colorTypeBox->addItem("z");
    colorTypeBox->addItem("intensity");
    colorTypeBox->addItem("rgb");
    colorTypeBox->addItem("random");
    colorTypeBox->setCurrentText(paramterConfig.getColorRenderType());

    pointSizeLabel = new QLabel(tr("显示点云大小:"));
    pointSizeBox = new QSpinBox();
    pointSizeBox->setMinimum(1);
    pointSizeBox->setMaximum(100);
    pointSizeBox->setValue(paramterConfig.getPointSize());
    pointSizeBox->setSingleStep(1);

    QHBoxLayout *layout2 = new QHBoxLayout();
    layout2->setSpacing(20);
    layout2->addWidget(colorTypeLabel);
    layout2->addWidget(colorTypeBox);
    layout2->addWidget(pointSizeLabel);
    layout2->addWidget(pointSizeBox);

    infoLabel = new QLabel(tr("注意：\n读取文件格式目前只支持pcd,bin,ply文件，\n"
                              "其中读取字段数参数只与读bin文件有关，\n"
                              "该参数描述了一个包含几个字段信息"));

    loadDefaultButton = new QPushButton(tr("恢复默认值"));
    saveButton = new QPushButton(tr("保存"));
    cancelButton = new QPushButton(tr("取消"));

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(30);
    bottomLayout->setAlignment(Qt::AlignRight);
    bottomLayout->addWidget(loadDefaultButton);
    bottomLayout->addWidget(saveButton);
    bottomLayout->addWidget(cancelButton);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(10);
    mainLayout->addLayout(layout0);
    mainLayout->addLayout(layout1);
    mainLayout->addWidget(isShowMeshBox);
    mainLayout->addLayout(layout2);
    mainLayout->addWidget(infoLabel);
    mainLayout->addLayout(bottomLayout);

    this->setLayout(mainLayout);
    this->setMaximumSize(480, 420);
    this->setMinimumSize(420, 420);
    this->setWindowTitle(tr("点云标注参数配置"));
}

void PointCloudMarkParamterWindow::initConnect()
{
    connect(loadDefaultButton, &QPushButton::clicked, this, &PointCloudMarkParamterWindow::loadDefaultValue);
    connect(saveButton, &QPushButton::clicked, this, &PointCloudMarkParamterWindow::slotOk);
    connect(cancelButton, &QPushButton::clicked, this, &PointCloudMarkParamterWindow::reject);
}

void PointCloudMarkParamterWindow::initFileType()
{
    formatBox->addItem("PCD", PointCloudFileType::PCD_FILE);
    formatBox->addItem("BIN", PointCloudFileType::BIN_FILE);
    formatBox->addItem("PLY", PointCloudFileType::PLY_FILE);
    // formatBox->addItem("OBJ", PointCloudFileType::OBJ_FILE);

    formatBox->setCurrentIndex(static_cast<int>(paramterConfig.getFileType()));
}

