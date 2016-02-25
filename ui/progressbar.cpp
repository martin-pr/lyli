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

#include "progressbar.h"

#include <QtWidgets/QProgressBar>

ProgressBar::ProgressBar(QWidget* parent) : QWidget(parent) {
	m_progressBar = new QProgressBar(this);
	m_progressBar->setEnabled(false);
	m_progressBar->setFormat(QStringLiteral("%v/%m"));
}

ProgressBar::~ProgressBar() {

}

void ProgressBar::onProgressStart(int files) {
	m_progressBar->setEnabled(true);
	m_progressBar->setMaximum(files);
}

void ProgressBar::onProgress(int progress) {
	m_progressBar->setValue(progress);
}

void ProgressBar::onProgressFinish() {
	m_progressBar->setEnabled(false);
}
