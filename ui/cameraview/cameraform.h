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

#ifndef CAMERAFORM_H
#define CAMERAFORM_H

#include <memory>

#include <QtCore/QThread>
#include <QtCore/QAbstractListModel>
#include <QtWidgets/QWidget>

namespace Ui {
class CameraForm;
}
class QModelIndex;

class Context;
class CameraListModel;
class ImageListModel;

class CameraForm : public QWidget {
	Q_OBJECT
public:
	CameraForm(QWidget *parent = nullptr);
	~CameraForm();

signals:
	void progressStart(int files);
	void progressRun(int progress);
	void progressFinish();

private:
	enum class DownloadMode {
		ALL,
		SELECTED
	};

private slots:
	void onCameraChanged(const QModelIndex& index);
	void onDownloadAll();
	void onDownloadSelected();

	void onDownloadStarted(int files);
	void onDownloadRunning(int progress);
	void onDownloadFinished();

private:
	std::unique_ptr<Context> m_context;
	std::unique_ptr<Ui::CameraForm> ui;

	CameraListModel* m_cameraListModel;
	ImageListModel* m_imageListModel;

	QThread m_downloadThread;

	/** Show the GUI for selecting the output directory and executes a download thread
	 */
	void download(DownloadMode mode);

	CameraForm(const CameraForm& other);
	CameraForm& operator=(const CameraForm& other);
};

#endif // CAMERAFORM_H
