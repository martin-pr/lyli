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

#include "cameralistmodel.h"

#include "context.h"

CameraListModel::CameraListModel(Context* context, QObject* parent): QAbstractListModel(parent), m_context(context) {

}

CameraListModel::~CameraListModel() {

}


QVariant CameraListModel::data(const QModelIndex& index, int role) const {
	if (! index.isValid() || static_cast<Lyli::CameraList::size_type>(index.row()) >= m_context->getCameraCount()) {
		return QVariant();
	}

	if (role == Qt::DisplayRole) {
		Lyli::CameraInformation info(m_context->getCamera(index.row())->getCameraInformation());
		QString name = QString(info.vendor.c_str()).trimmed() + " "
		               + QString(info.product.c_str()).trimmed() + " "
		               + QString(info.revision.c_str()).trimmed();
		return QVariant::fromValue(name);
	}

	return QVariant();
}

int CameraListModel::rowCount(const QModelIndex& parent) const {
	Q_UNUSED(parent);
	return m_context->getCameraCount();
}
