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


#ifndef VIEWERFORM_H
#define VIEWERFORM_H

#include <QtWidgets/QWidget>

#include "lytroimage.h"

namespace Ui {
class ViewerForm;
}

class QFileSystemModel;
class QModelIndex;

/**
 * An image viewer widget
 */
class ViewerForm : public QWidget {
	Q_OBJECT

public:
	ViewerForm(QWidget *parent = nullptr);
	~ViewerForm();

public slots:
	void directoryViewClicked(const QModelIndex &index);
	void fileViewClicked(const QModelIndex &index);

	void saveAs();

	void zoomIn();
	void zoomOut();
	void zoomFit();
	void zoomOriginal();

protected:
	virtual void resizeEvent(QResizeEvent *event);

private:
	Ui::ViewerForm* ui;
	QFileSystemModel *m_dirModel;
	QFileSystemModel *m_fileModel;

	double m_scale;
	LytroImage m_image;

	void scaleImage();

	ViewerForm(const ViewerForm& other);
	ViewerForm& operator=(const ViewerForm& other);
};

#endif // VIEWERFORM_H
