#ifndef BIRDVIEWPROCESS_H
#define BIRDVIEWPROCESS_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QHeaderView>
#include <QTableWidget>
#include <QImage>
#include "helpers/convertcvqimage.h"
#include "utilityGUI/customWindow/mytextbrowser.h"
#include "utilityGUI/customWindow/editnumberitemdelegate.h"
#include "imagelabel.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>

class BirdViewProcess : public QDialog
{
    Q_OBJECT
public:
    explicit BirdViewProcess(QDialog *parent = nullptr);
    ~BirdViewProcess();

signals:

public slots:

    void slotOpenImage();
    void slotSelectPoint();
    void slotBirdCalibration();
    void slotBirdView();
    void slotSaveResult();
    void slotComputeDistance();

    void slotAddBirdPoint(QPoint point);

private:

    ImageLabel *imageShow;
    QScrollArea *scrollArea;//滚动区域
    MyTextBrowser *commandText;//输出黑匣子指令

    QPushButton *openImageButton;
    QPushButton *selectPointButton;
    QPushButton *birdCalibrationButton;
    QPushButton *birdViewButton;
    QPushButton *saveResultButton;
    QPushButton *distanceButton;

    QTableWidget *birdPointTable;

    QGroupBox *paramGroundBox;
    QLabel *ratioLabel;
    QSpinBox *ratioBox;

    QImage currentImage;
    QImage birdViewImage;

    ConvertCVQImage convertImage;

    QString openDataDir;

    QList<QPoint> pointList;
    std::vector<cv::Point2f> birdPointList;
    cv::Mat homography;

    void init();
    void initUI();
    void initConnect();
};

#endif // BIRDVIEWPROCESS_H
