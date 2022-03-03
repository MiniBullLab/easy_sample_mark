#ifndef SELECTMARKCLASSWINDOW_H
#define SELECTMARKCLASSWINDOW_H
#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include "dataType/mark_data_type.h"

class SelectMarkTrackingWindow : public QDialog
{
     Q_OBJECT
public:
    explicit SelectMarkTrackingWindow(const MarkDataType dataType, QDialog *parent = 0);
    ~SelectMarkTrackingWindow();

    void setObjectRect(const QString sampleClass);
    QString getObjectClass();
    int getObjectId();
    bool getIsDifficult();
    int getObjectFlag();

signals:

public slots:

    void slotSelectOk(QListWidgetItem *item);
    void slotOk();
    void slotReset();

private:

    QLabel *classLabel;
    QComboBox *classBox;
    QLabel *idLable;
    QPushButton *addIdButton;
    QPushButton *resetIdButton;
    QListWidget *idListWidget;
    QLabel *objectFlagLabel;
    QGroupBox *flagGroundBox;
    QCheckBox *isDifficultBox;
    QComboBox *objectFlagBox;

    QList<QString> classList;

    QString sampleCalss;

    int objectId;

    MarkDataType markDataType;

    void initData();
    void initUI();
    void initConncet();

    void initObjectFlagBox();

    int loadConfig();
    int saveConfig();
};

#endif // SELECTMARKCLASSWINDOW_H
