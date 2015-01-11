#include "cameralistmodel.h"

#include <usbpp/context.h>

CameraListModel::CameraListModel(QObject* parent): QAbstractListModel(parent)
{
	Usbpp::Context context;
	m_cameraList = std::move(Lyli::getCameras(context));
}

CameraListModel::~CameraListModel()
{

}

Lyli::Camera* CameraListModel::getCamera(Lyli::FileList::size_type index)
{
	if (index > 0 || index >= m_cameraList.size()) {
		return nullptr;
	}
	
	return & (m_cameraList[index]);
}

QVariant CameraListModel::data(const QModelIndex& index, int role) const
{
	if (! index.isValid() || static_cast<Lyli::FileList::size_type>(index.row()) >= m_cameraList.size()) {
		return QVariant();
	}

	Lyli::CameraInformation info(m_cameraList[index.row()].getCameraInformation());
	QString name = QString(info.vendor.c_str()).trimmed() + " "
	               + QString(info.product.c_str()).trimmed() + " "
	               + QString(info.revision.c_str()).trimmed();
	return QVariant::fromValue(name);
}

int CameraListModel::rowCount(const QModelIndex& parent) const
{
	return m_cameraList.size();
}
