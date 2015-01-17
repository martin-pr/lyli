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

#include <QtGui/QPixmap>
#include <QtWidgets/QFileSystemModel>

#include "lytroimage.h"

ViewerForm::ViewerForm(QWidget *parent) : QWidget(parent)
{
	ui = new Ui::ViewerForm;
	ui->setupUi(this);
	
	m_dirModel = new QFileSystemModel;
	m_dirModel->setReadOnly(true);
	m_dirModel->setRootPath(QDir::currentPath());
	m_dirModel->setFilter(QDir::Dirs | QDir::NoDot);
	ui->directoryList->setModel(m_dirModel);
	ui->directoryList->setRootIndex(m_dirModel->index(QDir::currentPath()));
	connect(ui->directoryList, SIGNAL(activated(QModelIndex)), this, SLOT(directoryViewClicked(QModelIndex)));
	
	m_fileModel = new QFileSystemModel;
	m_fileModel->setReadOnly(true);
	m_fileModel->setRootPath(QDir::currentPath());
	m_fileModel->setFilter(QDir::Files);
	m_fileModel->setNameFilters(QStringList({"*.RAW"}));
	m_fileModel->setNameFilterDisables(false);
	ui->fileList->setModel(m_fileModel);
	ui->fileList->setRootIndex(m_fileModel->index(QDir::currentPath()));
	connect(ui->fileList, SIGNAL(activated(QModelIndex)), this, SLOT(fileViewClicked(QModelIndex)));
}

ViewerForm::~ViewerForm()
{
	delete ui;
}

void ViewerForm::directoryViewClicked(const QModelIndex& index)
{
	QString selected = m_dirModel->fileInfo(index).absoluteFilePath();
	ui->directoryList->setRootIndex(m_dirModel->setRootPath(selected));
	ui->fileList->setRootIndex(m_fileModel->setRootPath(selected));
}

void ViewerForm::fileViewClicked(const QModelIndex& index)
{
	// load the image
	LytroImage image(m_fileModel->fileInfo(index).absoluteFilePath().toLocal8Bit());
	ui->image->setPixmap(QPixmap::fromImage(*image.getQImage()));
}
