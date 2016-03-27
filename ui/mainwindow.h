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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>

#include "cameraview/cameraform.h"
#include "imageview/viewerform.h"
#include "progressbar.h"

namespace Ui {
class MainWindow;
}

/**
 * Main window
 */
class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();

signals:
	void windowShown();

protected:
	virtual void showEvent(QShowEvent *event) override;

private:
	Ui::MainWindow* ui;

	CameraForm *m_cameraForm;
	ProgressBar *m_progressBar;
	ViewerForm *m_viewerForm;
};

#endif // MAINWINDOW_H
