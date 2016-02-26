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

#include "viewerform.h"
#include "ui_viewerform.h"

#include <algorithm>
#include <QtCore/QByteArray>
#include <QtGui/QImageWriter>
#include <QtGui/QPixmap>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFileSystemModel>

#include "lytroimage.h"

ViewerForm::ViewerForm(QWidget *parent) : QWidget(parent), m_scale(1.0) {
	LytroImage::init();

	ui = new Ui::ViewerForm;
	ui->setupUi(this);

	m_dirModel = new QFileSystemModel;
	m_dirModel->setReadOnly(true);
	m_dirModel->setRootPath(QDir::currentPath());
	m_dirModel->setFilter(QDir::Dirs | QDir::NoDot);
	ui->directoryList->setModel(m_dirModel);
	ui->directoryList->setRootIndex(m_dirModel->index(QDir::currentPath()));
	connect(ui->directoryList, &QListView::activated, this, &ViewerForm::directoryViewClicked);

	m_fileModel = new QFileSystemModel;
	m_fileModel->setReadOnly(true);
	m_fileModel->setRootPath(QDir::currentPath());
	m_fileModel->setFilter(QDir::Files);
	m_fileModel->setNameFilters(QStringList({"*.RAW"}));
	m_fileModel->setNameFilterDisables(false);
	ui->fileList->setModel(m_fileModel);
	ui->fileList->setRootIndex(m_fileModel->index(QDir::currentPath()));
	connect(ui->fileList, &QListView::activated, this, &ViewerForm::fileViewClicked);

	connect(ui->buttonSaveAs, &QToolButton::clicked, this, &ViewerForm::saveAs);

	connect(ui->buttonZoomIn, &QToolButton::clicked, this, &ViewerForm::zoomIn);
	connect(ui->buttonZoomOut, &QToolButton::clicked, this, &ViewerForm::zoomOut);
	connect(ui->buttonZoomFit, &QToolButton::clicked, this, &ViewerForm::zoomFit);
	connect(ui->buttonZoomOriginal, &QToolButton::clicked, this, &ViewerForm::zoomOriginal);
}

ViewerForm::~ViewerForm() {
	delete ui;
}

void ViewerForm::directoryViewClicked(const QModelIndex& index) {
	QString selected = m_dirModel->fileInfo(index).absoluteFilePath();
	ui->directoryList->setRootIndex(m_dirModel->setRootPath(selected));
	ui->fileList->setRootIndex(m_fileModel->setRootPath(selected));
}

void ViewerForm::fileViewClicked(const QModelIndex& index) {
	// load the image
	m_image = std::move(LytroImage(m_fileModel->fileInfo(index).absoluteFilePath().toLocal8Bit()));
	ui->image->setPixmap(QPixmap::fromImage(*m_image.getQImage()));
}

void ViewerForm::saveAs() {
	// prepare filters based on the QImageWriter::supportedImageFormats();
	auto formats = QImageWriter::supportedImageFormats();
	QString filters;
	for (QByteArray format : formats) {
		filters.append(QString("%1 images (*.%2);;").arg(QString(format.toUpper()), QString(format.toLower())));
	}
	filters.append(tr("All Files (*)"));

	QString fileName = QFileDialog::getSaveFileName(this,
	                                                tr("Save File"),
	                                                QDir::currentPath(),
	                                                filters);

	if (! fileName.isEmpty()) {
		m_image.getQImage()->save(fileName);
	}
}

void ViewerForm::zoomIn() {
	m_scale *= 1.25;
	scaleImage();
}

void ViewerForm::zoomOut() {
	m_scale *= 0.75;
	scaleImage();
}

void ViewerForm::zoomFit() {
	m_scale = std::min((double) ui->scrollArea->size().width() / (double) ui->image->pixmap()->size().width(),
	                   (double) ui->scrollArea->size().height() / (double) ui->image->pixmap()->size().height());
	scaleImage();
}

void ViewerForm::zoomOriginal() {
	m_scale = 1.0;
	scaleImage();
}

void ViewerForm::resizeEvent(QResizeEvent *event) {
	scaleImage();
	QWidget::resizeEvent(event);
}

void ViewerForm::scaleImage() {
	if (ui->image->pixmap() != nullptr) {
		ui->image->resize(m_scale * ui->image->pixmap()->size());
		ui->scrollAreaWidgetContents->setMinimumSize(ui->image->size());
		ui->buttonZoomIn->setEnabled(m_scale < 3.0);
	}
}


