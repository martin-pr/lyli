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

#include "imagelistmodel.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QHash>
#include <QUrl>
#include <QVariant>

#include <algorithm>
#include <fstream>

#include "imagelistitem.h"

ImageListModel::ImageListModel(QObject *parent) : QAbstractListModel(parent)
{

}

ImageListModel::~ImageListModel()
{

}

QVariant ImageListModel::data(const QModelIndex& index, int role) const
{
	if (! index.isValid() || static_cast<Lyli::FileList::size_type>(index.row()) >= m_fileList.size()) {
		return QVariant();
	}
	
	if (role == Qt::DisplayRole) {
		return QVariant::fromValue(ImageListItem(m_camera, m_fileList[index.row()]));
	}
	
	return QVariant();
}

int ImageListModel::rowCount(const QModelIndex& parent) const
{
	return m_fileList.size();
}

void ImageListModel::changeCamera(Lyli::Camera* camera)
{
	if (m_camera != camera) {
		m_camera = camera;
		m_fileList = std::move(m_camera->getFileList());
		
		emit dataChanged(createIndex(0,0), createIndex(m_fileList.size(), 0));
	}
}

void ImageListModel::downloadFile(const QModelIndex &index, const QString &outputDirectory) {
	QString outputFile;
	QString outputFilePath;
	std::ofstream ofs;
	int id = index.row();

	QString outputFileBase =  outputDirectory + QDir::separator() + outputFile.sprintf("%04d", id);

	qDebug() << "downloading file " << outputFileBase;

	outputFilePath = outputFileBase + ".TXT";
	ofs.open(outputFilePath.toLocal8Bit().data(), std::ofstream::out | std::ofstream::binary);
	m_camera->getImageMetadata(ofs, id);
	ofs.flush();
	ofs.close();

	outputFilePath = outputFileBase + ".RAW";
	ofs.open(outputFilePath.toLocal8Bit().data(), std::ofstream::out | std::ofstream::binary);
	m_camera->getImageData(ofs, id);
	ofs.flush();
	ofs.close();
}
