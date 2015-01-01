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

#ifndef IMAGELIST_H
#define IMAGELIST_H

#include <QAbstractListModel>
#include <QObject>

#include <vector>

#include <camera.h>

class ImageListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ImageListRoles {
        IdRole = Qt::UserRole + 1,
        DateTimeRole,
        SelectRole
    };

    ImageListModel(Lyli::CameraList &cameraList, QObject *parent = 0);
    ~ImageListModel();

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    Q_INVOKABLE void setAdditiveSelection();
    Q_INVOKABLE void setRangeSelection();

    Q_INVOKABLE void selectRange(QModelIndex start, QModelIndex end, bool selected);
    Q_INVOKABLE void selectAll(bool selected);

    Q_INVOKABLE void downloadAll(const QUrl &outputDirectory);
    Q_INVOKABLE void downloadSelected(const QUrl &outputDirectory);

public slots:
    void changeCamera(int id);

private:
    Lyli::CameraList &m_cameraList;
    Lyli::Camera *m_camera;

    Lyli::FileList m_fileList;
    std::vector<bool> m_selected;

    enum class SelectionType {
        SINGLE,
        ADDITIVE,
        RANGE
    };

    SelectionType m_selectionType;
    // a last edited index used as the beginning for the range selection
    QModelIndex m_lastEditedIndex;

    void downloadFile(const QString &outputDirectory, int id);
};

#endif // IMAGELIST_H
