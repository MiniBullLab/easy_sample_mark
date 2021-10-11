#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "selectmarkocrwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>

#include "sampleMarkParam/manualparamterconfig.h"
#include "sampleMarkParam/segmentparamterconfig.h"

SelectMarkOCRWindow::SelectMarkOCRWindow(const MarkDataType dataType, QDialog *parent) :
    QDialog(parent), markDataType(dataType)
{
    initData();
    initUI();
    initConncet();
}

QString SelectMarkOCRWindow::getObjectText()
{
    return inputText->toPlainText();
}

QString SelectMarkOCRWindow::getLanguage()
{
    return this->languageBox->currentText();
}

bool SelectMarkOCRWindow::getIsIllegibility()
{
    return isIllegibilityBox->isChecked();
}

int SelectMarkOCRWindow::getObjectFlag()
{
    return this->objectFlagBox->currentData().toInt();
}

void SelectMarkOCRWindow::setObjectText(const QString &text)
{
    inputText->clear();
    inputText->setText(text);
}

void SelectMarkOCRWindow::setLanguage(const QString &language)
{
    languageBox->setCurrentText(language);
}

void SelectMarkOCRWindow::setIllegibility(const bool isIllegibility)
{
    isIllegibilityBox->setChecked(isIllegibility);
}

void SelectMarkOCRWindow::slotOk()
{
    QString tempStr = inputText->toPlainText();
    if(!isIllegibilityBox->isChecked() && tempStr.trimmed() == "")
    {
        QMessageBox::information(this, tr("字符"), tr("请请输入字符！"));
    }
    else
    {
        this->accept();

    }
}

void SelectMarkOCRWindow::initData()
{
    classList.clear();
    switch(markDataType)
    {
    case MarkDataType::IMAGE:
    case MarkDataType::VIDEO:
    case MarkDataType::SEGMENT:
    {
        QMap<QString, QString> markClassData = ManualParamterConfig::getMarkClass();
        for(QMap<QString, QString>::const_iterator classIterator = markClassData.constBegin();
            classIterator != markClassData.constEnd(); ++classIterator)
        {
            classList.append(classIterator.key());
        }
        break;
    }
    case MarkDataType::OCR:
        break;
    default:
        break;
    }
}

void SelectMarkOCRWindow::initUI()
{
    inputText = new QTextEdit();
    inputText->clear();
    inputText->append("#");
    okButton = new QPushButton(tr("确定"));
    cancelButton = new QPushButton(tr("取消"));
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(20);
    topLayout->addWidget(okButton);
    topLayout->addWidget(cancelButton);

    isIllegibilityBox = new QCheckBox(tr("是否模糊"));
    languageLable = new QLabel(tr("标注语言："));
    languageBox = new QComboBox();
    initLanguage();
    QHBoxLayout *languageLayout = new QHBoxLayout();
    languageLayout->setSpacing(10);
    languageLayout->addWidget(languageLable);
    languageLayout->addWidget(languageBox);
    QHBoxLayout *bottomLayout0 = new QHBoxLayout();
    bottomLayout0->setSpacing(10);
    bottomLayout0->addWidget(isIllegibilityBox);
    bottomLayout0->addLayout(languageLayout);

    objectFlagLabel = new QLabel(tr("目标备注："));
    objectFlagBox = new QComboBox();
    initObjectFlagBox();
    QHBoxLayout *bottomLayout1 = new QHBoxLayout();
    bottomLayout1->setSpacing(10);
    bottomLayout1->addWidget(objectFlagLabel);
    bottomLayout1->addWidget(objectFlagBox);

    flagGroundBox = new QGroupBox(tr("属性"));
    QVBoxLayout *flagLayout = new QVBoxLayout();
    flagLayout->setSpacing(10);
    flagLayout->addLayout(bottomLayout0);
    flagLayout->addLayout(bottomLayout1);
    flagGroundBox->setLayout(flagLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(20);
    mainLayout->addWidget(inputText);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(flagGroundBox);
    this->setLayout(mainLayout);
    this->setMaximumSize(400, 300);
    this->setMinimumSize(400, 300);
    this->setWindowTitle(tr("OCR文本输入"));
}

void SelectMarkOCRWindow::initConncet()
{
    connect(okButton, &QPushButton::clicked, this, &SelectMarkOCRWindow::slotOk);
    connect(cancelButton, &QPushButton::clicked, this, &SelectMarkOCRWindow::reject);
}

void SelectMarkOCRWindow::initLanguage()
{
    languageBox->addItem("english");
    languageBox->addItem("chinese");
}

void SelectMarkOCRWindow::initObjectFlagBox()
{
    for(int loop = 0; loop < 15; loop++)
    {
        objectFlagBox->addItem(QString("%1").arg(loop), loop);
    }
}
