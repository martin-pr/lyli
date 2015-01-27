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
	connect(ui->cameraList, &QListView::activated, this, &CameraForm::onChangeCamera);
	connect(ui->buttonDownloadAll, &QToolButton::clicked, this, &CameraForm::onDownloadAll);
	connect(ui->buttonDownloadSelected, &QToolButton::clicked, this, &CameraForm::onDownloadSelected);
	
	// some default settings
	if (ui->cameraList->model()->rowCount() > 0) {
		ui->cameraList->setCurrentIndex(ui->cameraList->model()->index(0,0));
		onChangeCamera(ui->cameraList->currentIndex());
	}
}

CameraForm::~CameraForm()
{
	m_downloadThread.quit();
	m_downloadThread.wait();
	delete ui;
}

void CameraForm::onChangeCamera(const QModelIndex &index)
{
	CameraListModel *model = static_cast<CameraListModel*>(ui->cameraList->model());
	Lyli::Camera *camera = model->getCamera(index.row());
	if (camera != nullptr) {
		ImageListModel *imageModel = static_cast<ImageListModel*>(ui->imageList->model());
		imageModel->changeCamera(camera);
	}
}

void CameraForm::onDownloadAll()
{
	download(DownloadMode::ALL);
}

void CameraForm::onDownloadSelected()
{
	download(DownloadMode::SELECTED);
}

void CameraForm::onDownloadStarted(int files)
{
	ui->buttonDownloadAll->setEnabled(false);
	ui->buttonDownloadSelected->setEnabled(false);
	emit progressStart(files);
}

void CameraForm::onDownloadRunning(int progress)
{
	emit progressRun(progress);
}

void CameraForm::onDownloadFinished()
{
	ui->buttonDownloadAll->setEnabled(true);
	ui->buttonDownloadSelected->setEnabled(true);
	emit progressFinish();
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
		connect(downloader, &ImageDownloader::started, this, &CameraForm::onDownloadStarted);
		connect(downloader, &ImageDownloader::progress, this, &CameraForm::onDownloadRunning);
		connect(downloader, &ImageDownloader::finished, this, &CameraForm::onDownloadFinished);
		
		connect(&m_downloadThread, &QThread::finished, downloader, &ImageDownloader::deleteLater);
		
		switch (mode) {
			case DownloadMode::ALL:
				connect(&m_downloadThread, &QThread::started, downloader, &ImageDownloader::onDownloadAll);
				break;
			case DownloadMode::SELECTED:
				connect(&m_downloadThread, &QThread::started, downloader, &ImageDownloader::onDownloadSelected);
				break;
		}
		
		m_downloadThread.start();
	}
}
