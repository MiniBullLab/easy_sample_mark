#ifndef IMAGETRACKINGCONTROLWINDOW_H
#define IMAGETRACKINGCONTROLWINDOW_H

#include "controlwindow.h"
#include "utilityGUI/customWindow/myscrollarea.h"
#include "drawWidget/tracking2dlabel.h"

class ImageTrackingControlWindow : public ControlWindow
{
    Q_OBJECT

public:
    ImageTrackingControlWindow(QWidget *parent = 0);
    virtual ~ImageTrackingControlWindow() override;

    void setMarkDataList(const QString markDataDir, const QList<QString> markDataList, const MarkDataType dataType) override;
    void saveMarkDataList() override;
    void setDrawShape(int shapeId) override;

public slots:

    void slotImageItem(QListWidgetItem *item);
    void slotChangeClass(QString classText);
    virtual void slotScrollArea(int keyValue);

protected:
    void closeEvent(QCloseEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;

protected:

    void isMarkData() override;
    void resetDraw() override;

    virtual void showPrevious();
    virtual void showNext();

    void updateDrawLabel(bool isValue);
    void updateImage();

    virtual void loadMarkData(const QString dataPath);
    virtual void saveMarkDataResult();
    virtual void loadMarkImage();

    void initDrawWidget();
    void initConnect();
    void initImageList();

protected:
    Tracking2DLabel *drawLable;
    QScrollArea *drawLableScrollArea;

private:
    //imageData
    void loadImageData(const QString imagePath, const QString saveAnnotationsDir);
    void saveImageDataResult(const QString &saveAnnotationsDir, const QString &imagePath,
                             const QList<MyObject> &objects);
    void initData();
    void initImageData();

private:
    //imageData
    QString currentImagePath;

};

#endif // IMAGETRACKINGCONTROLWINDOW_H
