#include "imagelistdelegate.h"

#include <QtCore/QRect>
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
	// the rectangle in which we have to draw and the upper left
	// corner that has to be used to offset into its position
	QRect rect(option.rect.topLeft() + QPoint(8,8), option.rect.bottomRight() - QPoint(8,8));
	QPoint ul(rect.topLeft());
	
	// drawing
	
	// border
	painter->drawRect(rect);
	// draw the thumbnail
	painter->drawImage(ul + QPoint(8, 8), item.getImage());
	// draw the date
	painter->drawText(ul + QPoint(8, 148), item.getTime().toString("d MMM yyyy"));
	// draw the time
	painter->drawText(ul + QPoint(8, 168), item.getTime().toString("hh:mm:ss"));
	
	painter->restore();
}

QSize ImageListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QSize(160, 200);
}

