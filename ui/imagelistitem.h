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

#ifndef IMAGELISTITEM_H
#define IMAGELISTITEM_H

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtGui/QImage>

#include <memory>

#include <camera.h>

class ImageListItem : public QObject
{
	Q_OBJECT

public:
    ImageListItem();
	ImageListItem(Lyli::Camera *camera, const Lyli::FileListEntry &fileEntry, QObject* parent = nullptr);
	ImageListItem(const ImageListItem& other);
	~ImageListItem();
	
	ImageListItem &operator=(const ImageListItem &other);
	
	bool isNull() const;
	
	int getId() const;
	QString getSha1() const;
	QDateTime getTime() const;
	QImage getImage() const;
	
	Q_PROPERTY(bool null READ isNull)
	Q_PROPERTY(int id READ getId)
	Q_PROPERTY(QString sha1 READ getSha1)
	Q_PROPERTY(QDateTime time READ getTime)
	Q_PROPERTY(QImage image READ getImage)

private:
	Lyli::Camera *m_camera;
	Lyli::FileListEntry m_fileEntry;
	// cached image
	mutable std::shared_ptr<QImage> m_image;
	
	static constexpr int IMG_WIDTH = 128;
	static constexpr int IMG_HEIGHT = 128;
};

Q_DECLARE_METATYPE(ImageListItem)

#endif // IMAGELISTITEM_H
