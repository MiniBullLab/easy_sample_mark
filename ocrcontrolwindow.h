#ifndef OCRCONTROLWINDOW_H
#define OCRCONTROLWINDOW_H

#include "controlwindow.h"
#include "utilityGUI/customWindow/myscrollarea.h"
#include "drawWidget/ocrlabel.h"

class OCRControlWindow : public ControlWindow
{
    Q_OBJECT

public:
    OCRControlWindow(QWidget *parent = 0);
    ~OCRControlWindow() override;

    void setMarkDataList(const QString markDataDir, const QList<QString> markDataList, const MarkDataType dataType) override;
    void saveMarkDataList() override;
    void setDrawShape(int shapeId) override;

public slots:

    void slotImageItem(QListWidgetItem *item);
    void slotChangeClass(QString classText);
    void slotScrollArea(int keyValue);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

protected:

    void isMarkData() override;
    void resetDraw() override;

    virtual void showPrevious();
    virtual void showNext();

    void updateDrawLabel(bool isValue);
    void updateImage();

    void loadMarkData(const QString dataPath);
    void saveMarkDataResult();
    void loadMarkImage();

    void initDrawWidget();
    void initConnect();
    void initImageList();

protected:
    OCRLabel *drawLable;
    QScrollArea *drawLableScrollArea;

private:
    //imageData
    void loadImageData(const QString &imagePath, const QString &saveAnnotationsDir);
    void saveImageDataResult(const QString &saveAnnotationsDir, const QString &imagePath,
                             const QList<MyObject> &objects);

    void initData();
    void initImageData();

private:
    //imageData
    QString currentImagePath;

};

#endif // OCRCONTROLWINDOW_H
