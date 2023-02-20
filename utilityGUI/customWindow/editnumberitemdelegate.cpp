#ifdef WIN32
#pragma execution_character_set("utf-8")
#endif
#include "editnumberitemdelegate.h"
#include <QDoubleSpinBox>

EditNumberItemDelegate::EditNumberItemDelegate(QObject *parent): QItemDelegate(parent)
{

}

QWidget *EditNumberItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setValue(0);
    editor->setMinimum(-10000);
    editor->setMaximum(10000);
    editor->setSingleStep(1);
    editor->setSuffix("m");
    editor->setAlignment(Qt::AlignCenter);
    return editor;
}

void EditNumberItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::EditRole).toDouble();
    QDoubleSpinBox *currentEditor = static_cast<QDoubleSpinBox*>(editor);
    currentEditor->setValue(value);
}

void EditNumberItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QDoubleSpinBox *currentEditor = static_cast<QDoubleSpinBox*>(editor);
    double value= currentEditor->value();
    //设置模型的数据
    model->setData(index, value, Qt::EditRole);
}

void EditNumberItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //根据option,设置编辑框位置
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
