#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "selectmarktrackingwindow.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

#include "sampleMarkParam/manualparamterconfig.h"

static int trackingId = 1;

SelectMarkTrackingWindow::SelectMarkTrackingWindow(const MarkDataType dataType, QDialog *parent) :
    QDialog(parent), markDataType(dataType)
{
    initData();
    initUI();
    initConncet();
}

SelectMarkTrackingWindow::~SelectMarkTrackingWindow()
{
    saveConfig();
}

QString SelectMarkTrackingWindow::getObjectClass()
{
    return classBox->currentText();
}

int SelectMarkTrackingWindow::getObjectId()
{
    return this->objectId;
}

bool SelectMarkTrackingWindow::getIsDifficult()
{
    return isDifficultBox->isChecked();
}

int SelectMarkTrackingWindow::getObjectFlag()
{
    return this->objectFlagBox->currentData().toInt();
}

void SelectMarkTrackingWindow::setObjectRect(const QString sampleClass)
{
    this->sampleCalss = sampleClass;
}

void SelectMarkTrackingWindow::slotSelectOk(QListWidgetItem *item)
{
    if(this->sampleCalss.contains("All") || classBox->currentText().contains(this->sampleCalss))
    {
        objectId = item->text().toInt();
        this->accept();
    }
    else
    {
        QMessageBox::information(this, tr("样本类别"), tr("您标注的样本类别有误！"));
    }
}

void SelectMarkTrackingWindow::slotOk()
{
    if(classBox->currentText().trimmed() == "")
    {
        QMessageBox::information(this, tr("样本类别"), tr("请选择样本类别！"));
    }
    else
    {
        objectId = trackingId;
        trackingId++;
        this->accept();

    }
}

void SelectMarkTrackingWindow::slotReset()
{
    trackingId = 1;
    idLable->setText(tr("目标当前ID:%1").arg(trackingId));
    idListWidget->clear();
    for(int index = 1; index < trackingId; index++)
    {
        QListWidgetItem *item = new QListWidgetItem(QString::number(index));
        idListWidget->insertItem(index, item);
    }
}

void SelectMarkTrackingWindow::initData()
{
    loadConfig();
    classList.clear();
    objectId = -1;
    switch(markDataType)
    {
    case MarkDataType::IMAGE:
    case MarkDataType::VIDEO:
    case MarkDataType::SEGMENT:
    {
        QMap<QString, QString> markClassData = ManualParamterConfig::getMarkClass();
        for(QMap<QString, QString>::const_iterator classIterator = markClassData.constBegin();
            classIterator != markClassData.constEnd(); ++classIterator)
        {
            classList.append(classIterator.key());
        }
        break;
    }
    default:
        break;
    }
}

void SelectMarkTrackingWindow::initUI()
{
    classLabel = new QLabel(tr("选择类别:"));
    classBox = new QComboBox();
    classBox->clear();
    for(int index = 0; index < classList.size(); index++)
    {
        classBox->addItem(classList[index]);
    }

    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(30);
    topLayout->addWidget(classLabel);
    topLayout->addWidget(classBox);

    idLable = new QLabel(tr("目标当前ID:%1").arg(trackingId));
    addIdButton = new QPushButton(tr("添加"));
    resetIdButton = new QPushButton(tr("重置"));
    idListWidget = new QListWidget();
    idListWidget->clear();
    for(int index = 1; index < trackingId; index++)
    {
        QListWidgetItem *item = new QListWidgetItem(QString::number(index));
        idListWidget->insertItem(index, item);
    }

    QHBoxLayout* centerLayout1 = new QHBoxLayout();
    centerLayout1->addWidget(idLable);
    centerLayout1->addWidget(addIdButton);
    centerLayout1->addWidget(resetIdButton);

    isDifficultBox = new QCheckBox(tr("是否是困难样本"));
    topLayout->setSpacing(20);
    isDifficultBox->setCheckable(false);

    objectFlagLabel = new QLabel(tr("目标备注："));
    objectFlagBox = new QComboBox();
    initObjectFlagBox();
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(20);
    bottomLayout->addWidget(objectFlagLabel);
    bottomLayout->addWidget(objectFlagBox);

    flagGroundBox = new QGroupBox(tr("属性"));
    QVBoxLayout *flagLayout = new QVBoxLayout();
    flagLayout->setSpacing(10);
    flagLayout->addWidget(isDifficultBox);
    flagLayout->addLayout(bottomLayout);
    flagGroundBox->setLayout(flagLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(20);
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(centerLayout1);
    mainLayout->addWidget(idListWidget);
    mainLayout->addWidget(flagGroundBox);
    this->setLayout(mainLayout);
    this->setMaximumSize(250, 400);
    this->setMinimumSize(250, 400);
    this->setWindowTitle(tr("样本类别选择"));
}

void SelectMarkTrackingWindow::initConncet()
{
    connect(idListWidget, &QListWidget::itemDoubleClicked, this, &SelectMarkTrackingWindow::slotSelectOk);
    connect(addIdButton, &QPushButton::clicked, this, &SelectMarkTrackingWindow::slotOk);
    connect(resetIdButton, &QPushButton::clicked, this, &SelectMarkTrackingWindow::slotReset);
}

void SelectMarkTrackingWindow::initObjectFlagBox()
{
    for(int loop = 0; loop < 15; loop++)
    {
        objectFlagBox->addItem(QString("%1").arg(loop), loop);
    }
}

int SelectMarkTrackingWindow::loadConfig()
{
    QByteArray data;
    QFile file;
    file.setFileName("./tracking_config.json");
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        return -1;
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
                QJsonObject jsonObject = parseDoucment.object();
                if(jsonObject.contains("trackingId"))
                {
                    trackingId = jsonObject.take("trackingId").toVariant().toInt();
                }
            }
        }
    }
    else
    {
        return -2;
    }
    return 0;
}

int SelectMarkTrackingWindow::saveConfig()
{
    QJsonDocument doc;
    QByteArray data;
    QJsonObject jsonData;
    QFile file("./tracking_config.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |QIODevice::Text))
    {
        return -1;
    }
    jsonData.insert("trackingId", QString::number(trackingId));
    doc.setObject(jsonData);
    data = doc.toJson();
    file.write(data);
    file.close();
    return 0;
}
