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

#include "imagelistitem.h"

#include <sstream>

ImageListItem::ImageListItem(): m_camera(nullptr)
{

}

ImageListItem::ImageListItem(Lyli::Camera *camera, const Lyli::FileListEntry &fileEntry, QObject* parent) :
	QObject(parent),
	m_camera(camera), m_fileEntry(fileEntry), m_image(std::make_shared<QImage>())
{
	
}

ImageListItem::ImageListItem(const ImageListItem& other) :
QObject(other.parent()), m_camera(other.m_camera), m_fileEntry(other.m_fileEntry), m_image(other.m_image)
{
	
}

ImageListItem::~ImageListItem()
{
	
}

ImageListItem& ImageListItem::operator=(const ImageListItem& other)
{
	m_camera = other.m_camera;
	m_fileEntry = other.m_fileEntry;
	m_image = other.m_image;

	return *this;
}

bool ImageListItem::isNull() const
{
	return m_camera == nullptr;
}

QDateTime ImageListItem::getTime() const
{
	if (m_camera == nullptr) {
		return QDateTime();
	}
	
	return QDateTime::fromTime_t(m_fileEntry.getTime());
}

QImage ImageListItem::getImage() const
{
	if (m_image && ! m_image->isNull()) {
		return *m_image;
	}
	
	*m_image = QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
	
	if (m_camera == nullptr) {
		m_image->fill(Qt::gray);
		return *m_image;
	}
	
	std::stringstream ss;
	m_fileEntry.getImageThumbnail()->download(ss);
	
	std::size_t pos(0);
	char buf[2];
	std::uint16_t tmpVal;
	int x, y;
	while (true) {
		if (pos / IMG_WIDTH >= IMG_HEIGHT) {
			break;
		}
		
		ss.read(buf, 2);
		
		tmpVal = * (uint16_t*)(buf);
		y = pos / IMG_WIDTH;
		x = pos % IMG_WIDTH;
		m_image->setPixel(x, y, qRgb(tmpVal, tmpVal, tmpVal));
		++pos;
	}
	
	return *m_image;
}

Lyli::FileListEntry &ImageListItem::getFileEntry()
{
	return m_fileEntry;
}
