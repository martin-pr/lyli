/*
 * This file is part of Lyli, an application to control Lytro camera
 * Copyright (C) 2016  Lukas Jirkovsky <l.jirkovsky @at@ gmail.com>
 *
 * Lyli is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, version 3 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QtWidgets/QWidget>

class QProgressBar;

class ProgressBar : public QWidget {
	Q_OBJECT;
public:
	ProgressBar(QWidget *parent = nullptr);
	~ProgressBar();
public slots:
	void onProgressStart(int files);
	void onProgress(int progress);
	void onProgressFinish();

private:
	QProgressBar* m_progressBar;
};

#endif // PROGRESSBAR_H
