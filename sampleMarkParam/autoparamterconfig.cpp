#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "autoparamterconfig.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QFile>
#include <QFileInfo>
#include <QVariant>

int AutoParamterConfig::inputDataWidth = 512;
int AutoParamterConfig::inputDataHeight = 512;
float AutoParamterConfig::threshold = 0.3f;
QMap<int, QString> AutoParamterConfig::modelLabels = QMap<int, QString>();

AutoParamterConfig::AutoParamterConfig()
{
    init();
}

AutoParamterConfig::~AutoParamterConfig()
{

}

void AutoParamterConfig::setInpuDataWidth(int width)
{
    inputDataWidth = width;
}

void AutoParamterConfig::setInpuDataHeight(int height)
{
    inputDataHeight = height;
}

void AutoParamterConfig::setThreshold(float value)
{
    threshold = value;
}

void AutoParamterConfig::setModelLabels(QMap<int, QString> labels)
{
    modelLabels.clear();
    modelLabels = labels;
}

int AutoParamterConfig::getInpuDataWidth()
{
    return inputDataWidth;
}

int AutoParamterConfig::getInpuDataHeight()
{
    return inputDataHeight;
}

float AutoParamterConfig::getThreshold()
{
    return threshold;
}

QMap<int, QString> AutoParamterConfig::getModelLabels()
{
    return modelLabels;
}

int AutoParamterConfig::loadConfig()
{
    QByteArray data;
    QFile file;
    file.setFileName("./auto_mark_config.json");
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
                if(jsonObject.contains("inputDataWidth"))
                {
                    inputDataWidth = jsonObject.take("inputDataWidth").toVariant().toInt();
                }

                if(jsonObject.contains("inputDataHeight"))
                {
                    inputDataHeight = jsonObject.take("inputDataHeight").toVariant().toInt();
                }

                if(jsonObject.contains("threshold"))
                {
                    threshold = jsonObject.take("threshold").toVariant().toFloat();
                }

                if(jsonObject.contains("labels"))
                {
                    modelLabels.clear();
                    QMap<QString, QVariant> readMarkClass = jsonObject.take("labels").toObject().toVariantMap();
                    for(QMap<QString, QVariant>::const_iterator iter = readMarkClass.constBegin();
                        iter != readMarkClass.constEnd(); ++iter)
                    {
                        modelLabels.insert(iter.key().toInt(), iter.value().toString());
                    }
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

int AutoParamterConfig::saveConfig()
{
    QMap<int, QString>::const_iterator classIterator;
    QMap<QString, QVariant> saveMarkClass;
    QJsonDocument doc;
    QByteArray data;
    QJsonObject jsonData;
    QFile file("./auto_mark_config.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |QIODevice::Text))
    {
        return -1;
    }
    for(classIterator = modelLabels.constBegin(); classIterator != modelLabels.constEnd(); ++classIterator)
    {
        saveMarkClass.insert(QString::number(classIterator.key()), QVariant(classIterator.value()));
    }
    jsonData.insert("inputDataWidth", QString::number(inputDataWidth));
    jsonData.insert("inputDataHeight", QString::number(inputDataHeight));
    jsonData.insert("threshold", QString::number(threshold));
    jsonData.insert("labels", QJsonObject::fromVariantMap(saveMarkClass));

    doc.setObject(jsonData);
    data = doc.toJson();
    file.write(data);
    file.close();
    return 0;
}

void AutoParamterConfig::init()
{

}
