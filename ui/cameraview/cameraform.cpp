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

#include <QtCore/QDir>
#include <QtCore/QModelIndex>
#include <QtWidgets/QFileDialog>

#include "cameralistmodel.h"
#include "context.h"
#include "imagedownloader.h"
#include "imagelistdelegate.h"
#include "imagelistmodel.h"

CameraForm::CameraForm(QWidget *parent) : QWidget(parent), m_context(new Context), ui(new Ui::CameraForm) {
	ui->setupUi(this);

	// set the models
	m_cameraListModel = new CameraListModel(m_context.get());
	m_imageListModel = new ImageListModel(m_context.get());
	ui->cameraList->setModel(m_cameraListModel);
	ui->imageList->setModel(m_imageListModel);

	// set the delegates
	ui->imageList->setItemDelegate(new ImageListDelegate);

	// connect signals & slots
	connect(ui->cameraList, &QListView::activated, this, &CameraForm::onCameraChanged);
	connect(ui->buttonDownloadAll, &QToolButton::clicked, this, &CameraForm::onDownloadAll);
	connect(ui->buttonDownloadSelected, &QToolButton::clicked, this, &CameraForm::onDownloadSelected);
	connect(m_context.get(), &Context::cameraChanged, m_imageListModel, &ImageListModel::onCameraChanged);

	// some default settings
	if (ui->cameraList->model()->rowCount() > 0) {
		ui->cameraList->setCurrentIndex(ui->cameraList->model()->index(0,0));
		onCameraChanged(ui->cameraList->currentIndex());
	}
}

CameraForm::~CameraForm() {
	m_downloadThread.quit();
	m_downloadThread.wait();
}

void CameraForm::onCameraChanged(const QModelIndex &index) {
	m_context->changeCurrentCamera(index.row());
}

void CameraForm::onDownloadAll() {
	download(DownloadMode::ALL);
}

void CameraForm::onDownloadSelected() {
	download(DownloadMode::SELECTED);
}

void CameraForm::onDownloadStarted(int files) {
	ui->buttonDownloadAll->setEnabled(false);
	ui->buttonDownloadSelected->setEnabled(false);
	emit progressStart(files);
}

void CameraForm::onDownloadRunning(int progress) {
	emit progressRun(progress);
}

void CameraForm::onDownloadFinished() {
	ui->buttonDownloadAll->setEnabled(true);
	ui->buttonDownloadSelected->setEnabled(true);
	emit progressFinish();
}

void CameraForm::download(DownloadMode mode) {
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
