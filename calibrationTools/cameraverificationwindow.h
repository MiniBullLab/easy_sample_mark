#ifndef CAMERAVERIFICATIONWINDOW_H
#define CAMERAVERIFICATIONWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QScrollArea>
#include <QImage>
#include <QGroupBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QListWidget>
#include <QCheckBox>
#include "helpers/convertcvqimage.h"
#include "utilityGUI/customWindow/mytextbrowser.h"
#include "imagelabel.h"
#include "calibration/camera_intrinsic/CalibrationHarp.hpp"

class CameraVerificationWindow : public QDialog
{
    Q_OBJECT
public:
    explicit CameraVerificationWindow(QDialog *parent = nullptr);
    ~CameraVerificationWindow();

signals:

public slots:
    void slotOpenImageDir();
    void slotImageItem(QListWidgetItem *item);
    void slotVerification();

private:
    void init();
    void initUI();
    void initConnect();

private:
    ImageLabel *imageShow;
    QScrollArea *scrollArea;//滚动区域
    MyTextBrowser *commandText;//输出黑匣子指令

    QListWidget *imageListWidget;
    QPushButton *openDirButton;
    QPushButton *verificationButton;

    QImage currentImage;
    QString currentImagePath;

    QList<QString> processDataList;
    QString openDataDir;

    CalibrationHarp distortionMeasurement;
};

#endif // CAMERAVERIFICATIONWINDOW_H
