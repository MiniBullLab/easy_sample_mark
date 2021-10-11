#ifndef OCROBJECT_H
#define OCROBJECT_H

#include <QString>
#include <QPolygon>

class OCRObject
{
public:
    OCRObject();
    ~OCRObject();

    QString objectText;
    QString language;
    bool isIllegibility;
};

#endif // OCROBJECT_H
