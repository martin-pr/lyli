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

#include "cameraform.h"
#include "ui_cameraform.h"

#include "cameralistmodel.h"
#include "imagedownloader.h"
#include "imagelistdelegate.h"
#include "imagelistmodel.h"

#include <QtCore/QDir>
#include <QtCore/QModelIndex>
#include <QtWidgets/QFileDialog>

CameraForm::CameraForm(QWidget *parent) : QWidget(parent)
{
	ui = new Ui::CameraForm;
	ui->setupUi(this);
	
	// set the models
	ui->cameraList->setModel(new CameraListModel);
	ui->imageList->setModel(new ImageListModel);
	
	// set the delegates
	ui->imageList->setItemDelegate(new ImageListDelegate);
	
	// connect signals & slots
	connect(ui->cameraList, SIGNAL(activated(QModelIndex)), this, SLOT(changeCamera(QModelIndex)));
	connect(ui->buttonDownloadAll, SIGNAL(clicked(bool)), this, SLOT(downloadAll()));
	connect(ui->buttonDownloadSelected, SIGNAL(clicked(bool)), this, SLOT(downloadSelected()));
}

CameraForm::~CameraForm()
{
	m_downloadThread.quit();
	m_downloadThread.wait();
	delete ui;
}

void CameraForm::changeCamera(const QModelIndex &index)
{
	CameraListModel *model = static_cast<CameraListModel*>(ui->cameraList->model());
	Lyli::Camera *camera = model->getCamera(index.row());
	if (camera != nullptr) {
		
		ImageListModel *imageModel = static_cast<ImageListModel*>(ui->imageList->model());
		imageModel->changeCamera(camera);
	}
}

void CameraForm::downloadAll()
{
	download(DownloadMode::ALL);
}

void CameraForm::downloadSelected()
{
	download(DownloadMode::SELECTED);
}

void CameraForm::downloadStarted(int files)
{
	ui->buttonDownloadAll->setEnabled(false);
	ui->buttonDownloadSelected->setEnabled(false);
}

void CameraForm::downloadProgress(int progress)
{

}

void CameraForm::downloadFinished()
{
	ui->buttonDownloadAll->setEnabled(true);
	ui->buttonDownloadSelected->setEnabled(true);
}

void CameraForm::download(DownloadMode mode)
{
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::Directory);
	dialog.setOption(QFileDialog::ShowDirsOnly, true);
	QString outputDirectory = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
	                                                            QDir::homePath(),
	                                                            QFileDialog::ShowDirsOnly);
	
	if (! outputDirectory.isEmpty()) {
		ImageDownloader *downloader = new ImageDownloader(qobject_cast<ImageListModel*>(ui->imageList->model()),
		                                                  ui->imageList->selectionModel()->selectedIndexes(),
		                                                  outputDirectory);
		downloader->moveToThread(&m_downloadThread);
		
		connect(&m_downloadThread, SIGNAL(finished()), downloader, SLOT(deleteLater()));
		
		switch (mode) {
			case DownloadMode::ALL:
				connect(&m_downloadThread, SIGNAL(started()), downloader, SLOT(downloadAll()));
				break;
			case DownloadMode::SELECTED:
				connect(&m_downloadThread, SIGNAL(started()), downloader, SLOT(downloadSelected()));
				break;
		}
		
		m_downloadThread.start();
	}
}
