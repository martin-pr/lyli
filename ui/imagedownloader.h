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

#ifndef IMAGEDOWNLOADER_H
#define IMAGEDOWNLOADER_H

#include <QtCore/QObject>

#include "imagelistmodel.h"

class ImageDownloader : public QObject {
	Q_OBJECT

public:
	ImageDownloader(ImageListModel *model, const QModelIndexList &indices, QString outputDir);
	virtual ~ImageDownloader();

public slots:
	void onDownloadAll();
	void onDownloadSelected();

signals:
	/** Signal emitted when the download starts.
	 *
	 * @param files the total number of files to be downloaded
	 */
	void started(int files);
	/** Signal emitted during the download to update the progress.
	 *
	 * @param progress the order of the currently downloaded file
	 */
	void progress(int progress);
	/** Signal emitted when the download finishes.
	 *
	 */
	void finished();

private:
	ImageListModel *m_model;
	QModelIndexList m_indices;
	QString m_outputDir;
};

#endif // IMAGEDOWNLOADER_H
