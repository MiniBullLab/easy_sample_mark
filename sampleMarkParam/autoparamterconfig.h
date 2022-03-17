#ifndef AUTOPARAMTERCONFIG_H
#define AUTOPARAMTERCONFIG_H

#include <QString>
#include <QMap>

class AutoParamterConfig
{
public:
    AutoParamterConfig();
    ~AutoParamterConfig();

    void setInpuDataWidth(int width);
    void setInpuDataHeight(int height);
    void setThreshold(float value);
    void setModelLabels(QMap<int, QString> labels);

    static int getInpuDataWidth();
    static int getInpuDataHeight();
    static float getThreshold();
    static QMap<int, QString> getModelLabels();
    static int loadConfig();
    static int saveConfig();

private:

    static int inputDataWidth;
    static int inputDataHeight;
    static float threshold;
    static QMap<int, QString> modelLabels;

private:

    void init();

};

#endif // AUTOPARAMTERCONFIG_H
