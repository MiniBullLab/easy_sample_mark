#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "autoparamterconfig.h"

int AutoParamterConfig::inputDataWidth = 512;
int AutoParamterConfig::inputDataHeight = 512;
float AutoParamterConfig::threshold = 0.1f;
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
    return 0;
}

int AutoParamterConfig::saveConfig()
{
    return 0;
}

void AutoParamterConfig::init()
{

}
