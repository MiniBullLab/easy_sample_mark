#ifndef POINTCLOUDPARAMTERCONFIG_H
#define POINTCLOUDPARAMTERCONFIG_H

#include <QString>

typedef enum PointCloudFileType{
    UNPCTYPE = -1,
    PCD_FILE = 0,
    BIN_FILE = 1,
    PLY_FILE = 2,
    OBJ_FILE = 3
}PointCloudFileType;

class PointCloudParamterConfig
{
public:
    PointCloudParamterConfig();
    ~PointCloudParamterConfig();

    void setPointSize(int size);
    void setFieldsNumber(int number);
    void setFileType(int type);
    void setIsMesh(bool is);

    static int getPointSize();
    static int getFieldsNumber();
    static bool getIsMesh();
    static PointCloudFileType getFileType();

    static int loadConfig();
    static int saveConfig();

private:
    static int POINT_SIZE;
    static int FIELDS_NUMBER;
    static bool IS_MESH;
    static PointCloudFileType FILE_TYPE;

private:
    void init();
};

#endif // POINTCLOUDPARAMTERCONFIG_H
