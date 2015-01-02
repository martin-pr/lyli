#include "thumbnailprovider.h"

#include <QImage>
#include <QSize>
#include <QString>

#include <sstream>

ThumbnailProvider::ThumbnailProvider() : QQuickImageProvider(QQmlImageProviderBase::Image),
	m_camera(nullptr)
{
	
}

ThumbnailProvider::~ThumbnailProvider()
{
	
}

QImage ThumbnailProvider::requestImage(const QString& id, QSize* size, const QSize& requestedSize)
{
	constexpr int IMG_WIDTH = 128;
	constexpr int IMG_HEIGHT = 128;
	
	if (m_camera == nullptr) {
		QImage image(requestedSize, QImage::Format_RGB32);
		image.fill(Qt::gray);
		return image;
	}
	
	QImage image(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
	std::stringstream ss;
	m_camera->getImageThumbnail(ss, id.toInt());
	
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
		image.setPixel(x, y, qRgb(tmpVal, tmpVal, tmpVal));
		++pos;
	}
	
	if (requestedSize.width() > 0 && requestedSize.width() != IMG_WIDTH ||
		requestedSize.height() > 0 && requestedSize.height() != IMG_HEIGHT) {
		image = image.scaled(requestedSize.width() > 0 ? requestedSize.width() : IMG_WIDTH,
		                       requestedSize.height() > 0 ? requestedSize.height() : IMG_HEIGHT,
		                       Qt::KeepAspectRatio);
	}
	if (size != nullptr) {
		*size = image.size();
	}
	
	return image;
}

void ThumbnailProvider::changeCamera(Lyli::Camera* camera)
{
	m_camera = camera;
}
