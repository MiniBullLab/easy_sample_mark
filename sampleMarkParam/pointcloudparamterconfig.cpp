#include "pointcloudparamterconfig.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QFile>
#include <QFileInfo>
#include <QVariant>

int PointCloudParamterConfig::POINT_SIZE = 2;
int PointCloudParamterConfig::FIELDS_NUMBER = 3;
QString PointCloudParamterConfig::COLOR_RENDER_TYPE = "x";
bool PointCloudParamterConfig::IS_MESH = false;
PointCloudFileType PointCloudParamterConfig::FILE_TYPE = PointCloudFileType::PCD_FILE;

PointCloudParamterConfig::PointCloudParamterConfig()
{

}


PointCloudParamterConfig::~PointCloudParamterConfig()
{

}

void PointCloudParamterConfig::setPointSize(int size)
{
    POINT_SIZE = size;
}

void PointCloudParamterConfig::setFieldsNumber(int number)
{
    FIELDS_NUMBER = number;
}

void PointCloudParamterConfig::setColorRenderType(const QString &type)
{
    COLOR_RENDER_TYPE = type;
}

void PointCloudParamterConfig::setFileType(int type)
{
    switch (type) {
    case -1:
        FILE_TYPE = PointCloudFileType::UNPCTYPE;
        break;
    case 0:
        FILE_TYPE = PointCloudFileType::PCD_FILE;
        break;
    case 1:
        FILE_TYPE = PointCloudFileType::BIN_FILE;
        break;
    case 2:
        FILE_TYPE = PointCloudFileType::PLY_FILE;
        break;
    case 3:
        FILE_TYPE = PointCloudFileType::OBJ_FILE;
        break;
    }
}

void PointCloudParamterConfig::setIsMesh(bool is)
{
    IS_MESH = is;
}

int PointCloudParamterConfig::getPointSize()
{
    return POINT_SIZE;
}

int PointCloudParamterConfig::getFieldsNumber()
{
    return FIELDS_NUMBER;
}

QString PointCloudParamterConfig::getColorRenderType()
{
    return COLOR_RENDER_TYPE;
}

bool PointCloudParamterConfig::getIsMesh()
{
    return IS_MESH;
}

PointCloudFileType PointCloudParamterConfig::getFileType()
{
    return FILE_TYPE;
}

int PointCloudParamterConfig::loadConfig()
{
    QByteArray data;
    QFile file;
    file.setFileName("./pointcloud_mark_config.json");
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
                if(jsonObject.contains("pointSize"))
                {
                    POINT_SIZE = jsonObject.take("pointSize").toVariant().toInt();
                }
                if(jsonObject.contains("fieldsNumber"))
                {
                    FIELDS_NUMBER = jsonObject.take("fieldsNumber").toVariant().toInt();
                }
                if(jsonObject.contains("colorRenderType"))
                {
                    COLOR_RENDER_TYPE = jsonObject.take("colorRenderType").toVariant().toString();
                }
                if(jsonObject.contains("isMesh"))
                {
                    IS_MESH = jsonObject.take("isMesh").toVariant().toBool();
                }
                if(jsonObject.contains("fileType"))
                {
                    int type = jsonObject.take("fileType").toVariant().toInt();
                    switch (type) {
                    case -1:
                        FILE_TYPE = PointCloudFileType::UNPCTYPE;
                        break;
                    case 0:
                        FILE_TYPE = PointCloudFileType::PCD_FILE;
                        break;
                    case 1:
                        FILE_TYPE = PointCloudFileType::BIN_FILE;
                        break;
                    case 2:
                        FILE_TYPE = PointCloudFileType::PLY_FILE;
                        break;
                    case 3:
                        FILE_TYPE = PointCloudFileType::OBJ_FILE;
                        break;
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

int PointCloudParamterConfig::saveConfig()
{
    QJsonDocument doc;
    QByteArray data;
    QJsonObject jsonData;
    QFile file("./pointcloud_mark_config.json");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate |QIODevice::Text))
    {
        return -1;
    }
    jsonData.insert("pointSize", QString::number(POINT_SIZE));
    jsonData.insert("fieldsNumber", QString::number(FIELDS_NUMBER));
    jsonData.insert("colorRenderType", COLOR_RENDER_TYPE);
    jsonData.insert("isMesh", QString::number(IS_MESH));
    jsonData.insert("fileType", QString::number(FILE_TYPE));

    doc.setObject(jsonData);
    data = doc.toJson();
    file.write(data);
    file.close();
    return 0;
}
