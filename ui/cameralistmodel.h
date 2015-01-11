#ifndef CAMERALISTMODEL_H
#define CAMERALISTMODEL_H

#include <QtCore/QAbstractListModel>

#include <camera.h>

class CameraListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	CameraListModel(QObject *parent=0);
	~CameraListModel();
	
	Lyli::Camera* getCamera(Lyli::FileList::size_type index);
	
	// inherited members
	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;

private:
	Lyli::CameraList m_cameraList;
};

#endif // CAMERALISTMODEL_H
