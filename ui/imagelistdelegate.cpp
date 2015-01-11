#include "imagelistdelegate.h"

#include <QtGui/QPainter>

#include "imagelistitem.h"

ImageListDelegate::ImageListDelegate(QObject* parent): QStyledItemDelegate(parent)
{

}

ImageListDelegate::~ImageListDelegate()
{

}

void ImageListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (! index.data().canConvert<ImageListItem>()) {
		return QStyledItemDelegate::paint(painter, option, index);
	}
	ImageListItem item = index.data().value<ImageListItem>();
	if (item.isNull()) {
		return QStyledItemDelegate::paint(painter, option, index);
	}
	
	painter->save();
	
	if (option.state & QStyle::State_Selected) {
		painter->setBrush(option.palette.highlight());
	}
	
	painter->setPen(Qt::black);
	painter->drawRect(option.rect);
	
	// draw the thumbnail
	painter->drawImage(QPoint(8, 8), item.getImage());
	// draw the date
	painter->drawText(QPoint(0, 142), item.getTime().toString("d MMM yyyy"));
	// draw the time
	painter->drawText(QPoint(0, 162), item.getTime().toString("hh:mm:ss"));
	
	painter->restore();
}

QSize ImageListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QSize(144, 184);
}

