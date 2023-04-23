#ifndef CAMERAINTRINSICSWINDOW_H
#define CAMERAINTRINSICSWINDOW_H

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
#include "calibration/camera_intrinsic/cameraintrinscalibration.h"

class CameraIntrinsicsWindow : public QDialog
{
    Q_OBJECT
public:
    explicit CameraIntrinsicsWindow(QDialog *parent = nullptr);
    ~CameraIntrinsicsWindow();

signals:

public slots:
    void slotOpenImageDir();
    void slotImageItem(QListWidgetItem *item);
    void slotCalibrate();
    void slotShowUndistortImage();
    void slotSaveCalibrateResult();

private:
    void init();
    void initUI();
    void initConnect();

private:
    QGroupBox *paramGroupBox;
    QLabel *boardSizeLabel;
    QSpinBox *boardSizeWidthBox;
    QSpinBox *boardSizeHeightBox;
    QLabel *squareSizeLabel;
    QSpinBox *squareSizeWidthBox;
    QSpinBox *squareSizeHeightBox;
    QLabel *cameraModelLabel;
    QComboBox *cameraModelBox;
    QCheckBox *isSaveUndistortBox;
    QCheckBox *isSelectCornersImageBox;

    ImageLabel *imageShow;
    QScrollArea *scrollArea;//滚动区域
    MyTextBrowser *commandText;//输出黑匣子指令

    QListWidget *imageListWidget;
    QPushButton *openDirButton;
    QPushButton *calibrationButton;
    QPushButton *undistortButton;
    QPushButton *saveResultButton;

    QImage currentImage;
    QString currentImagePath;

    QList<QString> processDataList;
    QString openDataDir;
    QString saveDir;

    CameraIntrinsCalibration calibrationProcess;
};

#endif // CAMERAINTRINSICSWINDOW_H
