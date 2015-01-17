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

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow()
{
	ui = new Ui::MainWindow;
	ui->setupUi(this);
	
	QHBoxLayout *cameraFormLayout = new QHBoxLayout;
	m_cameraForm = new CameraForm(ui->tabCamera);
	cameraFormLayout->setMargin(4);
	cameraFormLayout->addWidget(m_cameraForm);
	ui->tabCamera->setLayout(cameraFormLayout);
	
	QHBoxLayout *viewerFormLayout = new QHBoxLayout;
	m_viewerForm = new ViewerForm(ui->tabCamera);
	viewerFormLayout->setMargin(4);
	viewerFormLayout->addWidget(m_viewerForm);
	ui->tabViewer->setLayout(viewerFormLayout);
	
	m_progressBar = new QProgressBar;
	m_progressBar->setEnabled(false);
	ui->statusbar->addPermanentWidget(m_progressBar);
	
	connect(m_cameraForm, SIGNAL(progressStart(int)), this, SLOT(progressStart(int)));
	connect(m_cameraForm, SIGNAL(progressRun(int)), this, SLOT(progressStart(int)));
	connect(m_cameraForm, SIGNAL(progressStart(int)), this, SLOT(progressStart(int)));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::progressStart(int files)
{
	m_progressBar->setEnabled(true);
	m_progressBar->setMaximum(files);
}

void MainWindow::progressRun(int progress)
{
	m_progressBar->setValue(progress);
}

void MainWindow::progressFinish()
{
	m_progressBar->setEnabled(true);
}
