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

#ifndef CALIBRATIONWIZARD_H
#define CALIBRATIONWIZARD_H

#include <memory>

#include <QtWidgets/QWizard>
#include <QtWidgets/QWizardPage>

namespace Ui {
class PageStart;
class PageCacheDir;
class PageFinish;
}

/**
 * Wizard page with initial information message;
 */
class CalibrationWizardPageStart : public QWizardPage {
	Q_OBJECT
public:
	CalibrationWizardPageStart(QWidget *parent = nullptr);
	~CalibrationWizardPageStart();
	CalibrationWizardPageStart(const CalibrationWizardPageStart& other) = delete;
	CalibrationWizardPageStart& operator=(const CalibrationWizardPageStart& other) = delete;

private:
	std::unique_ptr<Ui::PageStart> m_ui;
};

/**
 * Wizard page with selection of the cache directory
 */
class CalibrationWizardPageCacheDir : public QWizardPage {
	Q_OBJECT
public:
	CalibrationWizardPageCacheDir(QWidget *parent = nullptr);
	~CalibrationWizardPageCacheDir();
	CalibrationWizardPageCacheDir(const CalibrationWizardPageCacheDir& other) = delete;
	CalibrationWizardPageCacheDir& operator=(const CalibrationWizardPageCacheDir& other) = delete;

public slots:
	void selectDirectory();

private:
	std::unique_ptr<Ui::PageCacheDir> m_ui;
};

/**
 * Wizard page with final information message;
 */
class CalibrationWizardPageFinish : public QWizardPage {
	Q_OBJECT
public:
	CalibrationWizardPageFinish(QWidget *parent = nullptr);
	~CalibrationWizardPageFinish();
	CalibrationWizardPageFinish(const CalibrationWizardPageStart& other) = delete;
	CalibrationWizardPageFinish& operator=(const CalibrationWizardPageStart& other) = delete;

private:
	std::unique_ptr<Ui::PageFinish> m_ui;
};

class CalibrationWizard : public QWizard {
	Q_OBJECT
public:
	CalibrationWizard(QWidget *parent = nullptr);
	~CalibrationWizard();
	CalibrationWizard(const CalibrationWizard& other) = delete;
	CalibrationWizard& operator=(const CalibrationWizard& other) = delete;
};

#endif // CALIBRATIONWIZARD_H
