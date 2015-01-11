#include "imagelistitem.h"

#include <sstream>

ImageListItem::ImageListItem(): m_camera(nullptr)
{

}

ImageListItem::ImageListItem(Lyli::Camera *camera, const Lyli::FileListEntry &fileEntry, QObject* parent) :
	QObject(parent), m_camera(camera), m_fileEntry(fileEntry)
{
	
}

ImageListItem::ImageListItem(const ImageListItem& other) :
QObject(other.parent()), m_camera(other.m_camera), m_fileEntry(other.m_fileEntry)
{
	if (! other.m_image.isNull()) {
		m_image = other.m_image;
	}
}

ImageListItem::~ImageListItem()
{
	
}

bool ImageListItem::isNull() const
{
	return m_camera == nullptr;
}

int ImageListItem::getId() const
{
	if (m_camera == nullptr) {
		return -1;
	}
	
	return m_fileEntry.id;
}

QString ImageListItem::getSha1() const
{
	if (m_camera == nullptr) {
		return QString();
	}
	
	QString result;
	result.setNum(m_fileEntry.sha1[0], 16);
	// simple but extremely ineffective
	for (int i = 1; i < 20; ++i) {
		QString tmp;
		tmp.setNum(m_fileEntry.sha1[i], 16);
		result += QStringLiteral(" ") + tmp;
	}
	return result;
}

QDateTime ImageListItem::getTime() const
{
	if (m_camera == nullptr) {
		return QDateTime();
	}
	
	return QDateTime::fromTime_t(m_fileEntry.time);
}

QImage ImageListItem::getImage() const
{
	constexpr int IMG_WIDTH = 128;
	constexpr int IMG_HEIGHT = 128;
	
	if (! m_image.isNull()) {
		return m_image;
	}
	
	if (m_camera == nullptr) {
		m_image = QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
		m_image.fill(Qt::gray);
		return m_image;
	}
	
	m_image = QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
	std::stringstream ss;
	m_camera->getImageThumbnail(ss, m_fileEntry.id);
	
	std::size_t pos(0);
	char buf[2];
	std::uint16_t tmpVal;
	int x, y;
	while (true) {
		if (pos / IMG_WIDTH >= IMG_HEIGHT) {
			break;
		}
		
		ss.read(buf, 2);
		
		tmpVal = * (uint16_t*)(buf);
		y = pos / IMG_WIDTH;
		x = pos % IMG_WIDTH;
		m_image.setPixel(x, y, qRgb(tmpVal, tmpVal, tmpVal));
		++pos;
	}
	
	return m_image;
}
