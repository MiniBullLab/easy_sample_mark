#ifndef LIDARCAMERACALIBRATWINDOW_H
#define LIDARCAMERACALIBRATWINDOW_H

#include <QDialog>


class LidarCameraCalibratWindow : public QDialog
{
    Q_OBJECT
public:
    explicit LidarCameraCalibratWindow(QDialog *parent = nullptr);
    ~LidarCameraCalibratWindow();

private:
    void init();
    void initUI();
    void initConnect();
};

#endif // LIDARCAMERACALIBRATWINDOW_H
