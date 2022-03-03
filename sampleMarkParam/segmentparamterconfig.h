#ifndef SEGMENTPARAMTERCONFIG_H
#define SEGMENTPARAMTERCONFIG_H

#include <QString>
#include <QList>
#include <QMap>

class SegmentParamterConfig
{
public:
    SegmentParamterConfig();
    ~SegmentParamterConfig();

    void setLineWidth(const int width);
    static int getLineWidth();

    void setSegmentPost(const std::string post);
    static QString getSegmentPost();

    static int loadConfig();
    static int saveConfig();

private:

    static int LINE_WIDTH;
    static std::string SEGMENT_POST;

private:

    void init();
};

#endif // SEGMENTPARAMTERCONFIG_H
