#ifndef BIRDVIEWPROCESS_H
#define BIRDVIEWPROCESS_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QScrollArea>
#include <QGroupBox>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QImage>
#include "helpers/convertcvqimage.h"
#include "utilityGUI/customWindow/mytextbrowser.h"
#include "imagelabel.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ccalib.hpp>
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
    void slotComputeDistance();

private:

    ImageLabel *imageShow;
    QScrollArea *scrollArea;//滚动区域
    MyTextBrowser *commandText;//输出黑匣子指令

    QPushButton *openImageButton;

    QGroupBox *paramGroundBox;
    QLabel *boxWidthLable;
    QDoubleSpinBox *boxWidthBox;
    QLabel *boxHeightLable;
    QDoubleSpinBox *boxHeightBox;
    QLabel *ratioLabel;
    QSpinBox *ratioBox;

    QLabel *chessboardSizeLabel;
    QDoubleSpinBox *chessboardSizeBox;
    QLabel *chessboardCountLabel;
    QSpinBox *chessboardWBox;
    QSpinBox *chessboardHBox;

    QPushButton *selectPointButton;
    QPushButton *birdCalibrationButton;
    QPushButton *birdViewButton;
    QPushButton *distanceButton;

    QImage currentImage;
    QImage birdViewImage;

    ConvertCVQImage convertImage;

    std::vector<std::vector<cv::Point2f>> allCorners;
    cv::Mat homography;

    void init();
    void initUI();
    void initConnect();
};

#endif // BIRDVIEWPROCESS_H
