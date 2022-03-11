#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "autoparamterconfigwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <limits>

AutoParamterConfigWindow::AutoParamterConfigWindow(QDialog *parent) : QDialog(parent), paramterConfig()
{
    init();
    initUI();
    initConnect();
}

AutoParamterConfigWindow::~AutoParamterConfigWindow()
{

}

void AutoParamterConfigWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton result = QMessageBox::question(this, tr("保存自动标注参数配置信息"), tr("是否保存配置信息?"),
                                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if(result == QMessageBox::Yes)
    {
        slotOk();
    }
    QDialog::closeEvent(event);
}

void AutoParamterConfigWindow::slotOk()
{
    QMap<int, QString> modelLabelsData = this->modelLabelTable->getModelLabels();
    if(modelLabelsData.size() <= 0)
    {
        QMessageBox::information(this, tr("自动标注参数设置"), tr("自动标注参数设置有误！"));
    }
    else
    {
        paramterConfig.setInpuDataWidth(this->inputDataWidthBox->value());
        paramterConfig.setInpuDataHeight(this->inputDataHeightBox->value());
        paramterConfig.setModelLabels(modelLabelsData);
        paramterConfig.saveConfig();
        this->accept();
    }
}

void AutoParamterConfigWindow::init()
{
    paramterConfig.loadConfig();
}

void AutoParamterConfigWindow::initUI()
{
    inputDataWidthLabel = new QLabel(tr("模型输入图象宽度"));
    inputDataWidthBox = new QSpinBox();
    inputDataWidthBox->setSingleStep(10);
    inputDataWidthBox->setMinimum(1);
    inputDataWidthBox->setMaximum(INT_MAX);
    inputDataWidthBox->setValue(paramterConfig.getInpuDataWidth());

    inputDataHeightLabel = new QLabel(tr("模型输入图象高度"));
    inputDataHeightBox = new QSpinBox();
    inputDataHeightBox->setSingleStep(10);
    inputDataHeightBox->setMinimum(1);
    inputDataHeightBox->setMaximum(INT_MAX);
    inputDataHeightBox->setValue(paramterConfig.getInpuDataHeight());

    QHBoxLayout *topLayout1 = new QHBoxLayout();
    topLayout1->setSpacing(10);
    topLayout1->addWidget(inputDataWidthLabel);
    topLayout1->addWidget(inputDataWidthBox);
    topLayout1->addWidget(inputDataHeightLabel);
    topLayout1->addWidget(inputDataHeightBox);

    thresholdLabel = new QLabel(tr("置信度:"));
    thresholdBox = new QDoubleSpinBox();
    thresholdBox->setSingleStep(0.1);
    thresholdBox->setMaximum(0);
    thresholdBox->setValue((double)paramterConfig.getThreshold());

    QHBoxLayout *topLayout2 = new QHBoxLayout();
    topLayout2->setSpacing(30);
    topLayout2->addWidget(thresholdLabel);
    topLayout2->addWidget(thresholdBox);

    modelLabelTable = new ModelLabelTableWidget();
    initTable();

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
    mainLayout->addLayout(topLayout1);
    mainLayout->addLayout(topLayout2);
    mainLayout->addWidget(modelLabelTable);
    mainLayout->addLayout(bottomLayout);

    this->setLayout(mainLayout);
    this->setMaximumSize(420, 500);
    this->setMinimumSize(420, 500);
    this->setWindowTitle(tr("自动标注参数配置"));
}

void AutoParamterConfigWindow::initConnect()
{
    connect(loadDefaultButton, &QPushButton::clicked, this, &AutoParamterConfigWindow::loadDefaultValue);
    connect(saveButton, &QPushButton::clicked, this, &AutoParamterConfigWindow::slotOk);
    connect(cancelButton, &QPushButton::clicked, this, &AutoParamterConfigWindow::reject);
}

void AutoParamterConfigWindow::initTable()
{
    int row = 0;
    QMap<int, QString>::const_iterator iterator;
    QStringList headerName;
    QMap<int, QString> modelLabelsData = paramterConfig.getModelLabels();
    int rows = modelLabelsData.count();
    modelLabelTable->clear();
    modelLabelTable->setRowCount(rows);
    modelLabelTable->setColumnCount(2);
    headerName << QString(tr("标签ID")) << QString(tr("标签名称"));
    modelLabelTable->setHorizontalHeaderLabels(headerName);
    modelLabelTable->horizontalHeader()->setEnabled(false);
    modelLabelTable->verticalHeader()->setVisible(false);
    modelLabelTable->horizontalHeader()->setStretchLastSection(true);
    modelLabelTable->horizontalHeader()->resizeSection(0,150); //设置表头第一列的宽度为150
    modelLabelTable->horizontalHeader()->setFixedHeight(25); //设置表头的高度
    modelLabelTable->setSelectionBehavior(QTableWidget::SelectRows);
    modelLabelTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    modelLabelTable->setFrameShape(QFrame::NoFrame); //设置无边框
    modelLabelTable->setSelectionMode(QAbstractItemView::SingleSelection);
    for(iterator = modelLabelsData.constBegin(); iterator != modelLabelsData.constEnd(); ++iterator)
    {
        QTableWidgetItem* tableItem0 = new QTableWidgetItem(QString::number(iterator.key()));
        modelLabelTable->setItem(row, 0, tableItem0);
        QTableWidgetItem* tableItem1 = new QTableWidgetItem(iterator.value());
        modelLabelTable->setItem(row, 1, tableItem1);
        row++;
    }
}

void AutoParamterConfigWindow::loadDefaultValue()
{
    int row = 0;
    QMap<int, QString>::const_iterator iterator;
    QMap<int, QString> modelLabelsData;
    QStringList headerName;
    modelLabelsData.clear();
    modelLabelsData.insert(0, "background");
    modelLabelsData.insert(2, "bicycle");
    modelLabelsData.insert(6, "bus");
    modelLabelsData.insert(7, "car");
    modelLabelsData.insert(14, "motorbike");
    modelLabelsData.insert(15, "person");
    int rows = modelLabelsData.count();
    modelLabelTable->clear();
    modelLabelTable->setRowCount(rows);
    modelLabelTable->setColumnCount(2);
    headerName << QString(tr("标签ID")) << QString(tr("标签名称"));
    modelLabelTable->setHorizontalHeaderLabels(headerName);
    for(iterator = modelLabelsData.constBegin(); iterator != modelLabelsData.constEnd(); ++iterator)
    {
        QTableWidgetItem* tableItem0 = new QTableWidgetItem(QString::number(iterator.key()));
        modelLabelTable->setItem(row, 0, tableItem0);
        QTableWidgetItem* tableItem1 = new QTableWidgetItem(iterator.value());
        modelLabelTable->setItem(row, 1, tableItem1);
        row++;
    }
    inputDataWidthBox->setValue(512);
    inputDataHeightBox->setValue(512);
}
