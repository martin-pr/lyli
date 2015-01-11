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

#ifndef IMAGELIST_H
#define IMAGELIST_H

#include <QtCore/QAbstractListModel>
#include <QObject>

#include <camera.h>

class ImageListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	ImageListModel(QObject *parent = 0);
	~ImageListModel();

	// inherited members
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

public slots:
	void changeCamera(Lyli::Camera *camera);
	void downloadFile(const QModelIndex &index, const QString &outputDirectory);

private:
	Lyli::Camera *m_camera;
	Lyli::FileList m_fileList;
};

#endif // IMAGELIST_H
