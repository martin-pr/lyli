#include "cameralistmodel.h"

Usbpp::Context CameraListModel::m_context;

CameraListModel::CameraListModel(QObject* parent): QAbstractListModel(parent)
{
	m_cameraList = std::move(Lyli::getCameras(m_context));
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

	if (role == Qt::DisplayRole) {
		Lyli::CameraInformation info(m_cameraList[index.row()].getCameraInformation());
		QString name = QString(info.vendor.c_str()).trimmed() + " "
		               + QString(info.product.c_str()).trimmed() + " "
		               + QString(info.revision.c_str()).trimmed();
		return QVariant::fromValue(name);
	}
	
	return QVariant();
}

int CameraListModel::rowCount(const QModelIndex& parent) const
{
	return m_cameraList.size();
}
