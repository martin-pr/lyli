#ifndef IMAGELISTDELEGATE_H
#define IMAGELISTDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>

class ImageListDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	
public:
    explicit ImageListDelegate(QObject* parent = nullptr);
    virtual ~ImageListDelegate();
	
    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
};

#endif // IMAGELISTDELEGATE_H
