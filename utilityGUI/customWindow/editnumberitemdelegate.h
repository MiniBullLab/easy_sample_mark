#ifndef EDITNUMBERITEMDELEGATE_H
#define EDITNUMBERITEMDELEGATE_H

#include <QItemDelegate>

class EditNumberItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    EditNumberItemDelegate(QObject *parent = nullptr);

public:
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // EDITNUMBERITEMDELEGATE_H
