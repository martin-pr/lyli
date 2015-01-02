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

#include "imagelistmodel.h"
#include "thumbnailprovider.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QHash>
#include <QUrl>
#include <QVariant>

#include <algorithm>
#include <fstream>

ImageListModel::ImageListModel(Lyli::CameraList &cameraList, ThumbnailProvider *thumbnailProvider, QObject *parent) :
	QAbstractListModel(parent),
    m_cameraList(cameraList),
    m_thumbnailProvider(thumbnailProvider),
    m_camera(nullptr),
    m_selectionType(SelectionType::SINGLE)
{

}

ImageListModel::~ImageListModel()
{

}

void ImageListModel::changeCamera(int id) {
    m_camera = & m_cameraList[id];
	m_thumbnailProvider->changeCamera(m_camera);

    m_fileList = std::move(m_camera->getFileList());
    m_selected = std::move(std::vector<bool>(m_fileList.size(), false));

    // notify that everything has changed
    emit dataChanged(createIndex(0, 0), createIndex(m_selected.size() - 1, 0));
}

Qt::ItemFlags ImageListModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QHash<int, QByteArray> ImageListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole] = "id";
    roles[DateTimeRole] = "datetime";
    roles[SelectRole] = "selected";
    return roles;
}

int ImageListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_fileList.size();
}

QVariant ImageListModel::data(const QModelIndex &index, int role) const {
    if (! index.isValid() || static_cast<Lyli::FileList::size_type>(index.row()) >= m_fileList.size()) {
        return QVariant();
    }

    Lyli::FileListEntry entry(m_fileList[index.row()]);
    switch (role) {
        case Qt::DisplayRole:
        case IdRole:
            return QVariant::fromValue(entry.id);
        case DateTimeRole:
            return QVariant::fromValue(QDateTime::fromTime_t(entry.time));
        case SelectRole:
            return QVariant::fromValue(m_selected[index.row()]);
        default:
            return QVariant();
    }
}

bool ImageListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && static_cast<Lyli::FileList::size_type>(index.row()) < m_fileList.size()) {
        switch (role) {
            case SelectRole:
                if (m_selectionType != SelectionType::ADDITIVE) {
                    selectAll(false);
                }
                if (m_selectionType == SelectionType::RANGE) {
                    selectRange(m_lastEditedIndex, index, value.toBool());
                }
                else {
                    m_selected[index.row()] = value.toBool();
                    emit dataChanged(index, index);
                }
                // store the currently edited index so we can use it if the user
                // requests the range selection. It can be done here because it is called whenever
                // the user clicks on an item
                m_lastEditedIndex = index;

                // reset the selection type, because the selection type is set on each click again
                m_selectionType = SelectionType::SINGLE;

                return true;
            default:
                return false;
        }
    }
    return false;
}

void ImageListModel::setAdditiveSelection() {
    m_selectionType = SelectionType::ADDITIVE;
}

void ImageListModel::setRangeSelection() {
    m_selectionType = SelectionType::RANGE;
}

void ImageListModel::selectRange(QModelIndex start, QModelIndex end, bool selected) {
    if (start.row() > end.row()) {
        std::swap(start, end);
    }
    auto itStart(std::next(m_selected.begin(), start.row()));
    auto itEnd(std::next(m_selected.begin(), end.row() + 1));
    std::fill(itStart, itEnd, selected);
    emit dataChanged(start, end);
}

void ImageListModel::selectAll(bool selected) {
    std::fill(m_selected.begin(), m_selected.end(), selected);
    emit dataChanged(createIndex(0, 0), createIndex(m_selected.size() - 1, 0));
}

void ImageListModel::downloadAll(const QUrl &outputDirectory) {
    for (Lyli::FileListEntry entry : m_fileList) {
        downloadFile(outputDirectory.toLocalFile(), entry.id);
    }
}

void ImageListModel::downloadSelected(const QUrl &outputDirectory) {
    for (std::size_t i(0); i < m_fileList.size(); ++i) {
        if (m_selected[i]) {
            downloadFile(outputDirectory.toLocalFile(), m_fileList[i].id);
        }
    }
}

void ImageListModel::downloadFile(const QString &outputDirectory, int id) {
    QString outputFile;
    QString outputFilePath;
    std::ofstream ofs;

    QString outputFileBase =  outputDirectory + QDir::separator() + outputFile.sprintf("%04d", id);

    qDebug() << "downloading file " << outputFileBase;

    outputFilePath = outputFileBase + ".TXT";
    ofs.open(outputFilePath.toLocal8Bit().data(), std::ofstream::out | std::ofstream::binary);
    m_camera->getImageMetadata(ofs, id);
    ofs.flush();
    ofs.close();

    outputFilePath = outputFileBase + ".RAW";
    ofs.open(outputFilePath.toLocal8Bit().data(), std::ofstream::out | std::ofstream::binary);
    m_camera->getImageData(ofs, id);
    ofs.flush();
    ofs.close();
}
