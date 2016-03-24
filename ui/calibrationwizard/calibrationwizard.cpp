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
 *
 */

#include "calibrationwizard.h"

#include <QtCore/QStandardPaths>
#include <QtWidgets/QFileDialog>

#include "ui_pagestart.h"
#include "ui_pagecachedir.h"
#include "ui_pagefinish.h"

CalibrationWizardPageStart::CalibrationWizardPageStart(QWidget *parent) : QWizardPage(parent) {
	m_ui = std::make_unique<Ui::PageStart>();
	m_ui->setupUi(this);
}

CalibrationWizardPageStart::~CalibrationWizardPageStart() {

}

CalibrationWizardPageCacheDir::CalibrationWizardPageCacheDir(QWidget *parent) : QWizardPage(parent) {
	m_ui = std::make_unique<Ui::PageCacheDir>();
	m_ui->setupUi(this);

	registerField("cacheDir*", m_ui->path);

	QString defaultCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	m_ui->path->setText(defaultCacheDir);

	connect(m_ui->pathSelect, &QPushButton::clicked, this, &CalibrationWizardPageCacheDir::selectDirectory);
}

CalibrationWizardPageCacheDir::~CalibrationWizardPageCacheDir() {

}

void CalibrationWizardPageCacheDir::selectDirectory() {
	QString defaultCacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	QString cacheDir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), defaultCacheDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	m_ui->path->setText(cacheDir);
}

CalibrationWizardPageFinish::CalibrationWizardPageFinish(QWidget *parent) : QWizardPage(parent) {
	m_ui = std::make_unique<Ui::PageFinish>();
	m_ui->setupUi(this);
}

CalibrationWizardPageFinish::~CalibrationWizardPageFinish() {

}

CalibrationWizard::CalibrationWizard(QWidget *parent) : QWizard(parent) {
	addPage(new CalibrationWizardPageStart);
	addPage(new CalibrationWizardPageCacheDir);
	addPage(new CalibrationWizardPageFinish);

	setWindowTitle(tr("Calibration Wizard"));
}

CalibrationWizard::~CalibrationWizard() {

}
