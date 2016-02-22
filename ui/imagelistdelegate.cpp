/*
 * This file is part of Lyli-Qt, a GUI to control Lytro camera
 * Copyright (C) 2015  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "imagelistdelegate.h"

#include <QtCore/QRect>
#include <QtGui/QPainter>

#include "imagelistitem.h"

ImageListDelegate::ImageListDelegate(QObject* parent): QStyledItemDelegate(parent) {

}

ImageListDelegate::~ImageListDelegate() {

}

void ImageListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
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

QSize ImageListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
	Q_UNUSED(option);
	Q_UNUSED(index);
	return QSize(160, 200);
}

