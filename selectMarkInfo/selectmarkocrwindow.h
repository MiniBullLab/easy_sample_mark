#ifndef SELECTMARKOCRWINDOW_H
#define SELECTMARKOCRWINDOW_H
#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QListWidget>
#include <QGroupBox>
#include "dataType/mark_data_type.h"

class SelectMarkOCRWindow : public QDialog
{
     Q_OBJECT
public:
    explicit SelectMarkOCRWindow(const MarkDataType dataType, QDialog *parent = 0);

    QString getObjectText();
    QString getLanguage();
    bool getIsIllegibility();
    int getObjectFlag();

    void setObjectText(const QString &text);
    void setLanguage(const QString &language);
    void setIllegibility(const bool isIllegibility);

signals:

public slots:

    void slotOk();

private:

    QTextEdit *inputText;
    QPushButton *okButton;
    QPushButton *cancelButton;
    QGroupBox *flagGroundBox;
    QCheckBox *isIllegibilityBox;
    QLabel *languageLable;
    QComboBox *languageBox;
    QLabel *objectFlagLabel;
    QComboBox *objectFlagBox;

    QList<QString> classList;

    QString sampleCalss;

    MarkDataType markDataType;

    void initData();
    void initUI();
    void initConncet();

    void initLanguage();
    void initObjectFlagBox();
};

#endif // SELECTMARKOCRWINDOW_H
