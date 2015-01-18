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

#include "imagedownloader.h"

ImageDownloader::ImageDownloader(ImageListModel* model, const QModelIndexList& indices, QString outputDir):
	m_model(model), m_indices(indices), m_outputDir(outputDir)
{
}

ImageDownloader::~ImageDownloader()
{
}

void ImageDownloader::onDownloadAll()
{
	const int rowCount = m_model->rowCount();
	
	emit started(rowCount);
	
	// download all files
	for (int i = 0; i < rowCount; ++i) {
		QModelIndex index = m_model->index(i, 0);
		m_model->downloadFile(index, m_outputDir);
		
		emit progress(i+1);
	}
	
	emit finished();
}

void ImageDownloader::onDownloadSelected()
{
	emit started(m_indices.size());
	
	// download selected files
	int i = 0;
	foreach(const QModelIndex &index, m_indices) {
		m_model->downloadFile(index, m_outputDir);
		
		emit progress(i+1);
		++i;
	}
	
	emit finished();
}

