#include "lytroimage.h"

#include <QtGui/QImage>

#include <cstddef>
#include <cstdint>
#include <fstream>

#include <rawimage.h>

namespace {

constexpr std::size_t IMG_WIDTH=3280;
constexpr std::size_t IMG_HEIGHT=3280;

}

LytroImage::LytroImage(const char *file)
{
	m_image = new QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
	std::fstream fin(file, std::fstream::in | std::fstream::binary);
	
	Lyli::RawImage image(fin, IMG_WIDTH, IMG_HEIGHT);
	fin.close();
	
	uint16_t *rawImage = image.getData();
	// combine the images
	std::size_t pos(0);
	for(std::size_t y = 0; y < IMG_HEIGHT; ++y) {
		for(std::size_t x = 0; x < IMG_WIDTH; ++x) {
			m_image->setPixel(x, y, qRgb(
				rawImage[pos],
				rawImage[pos + 1],
				rawImage[pos + 2]));
			pos += 3;
		}
	}
}

LytroImage::~LytroImage()
{
	delete m_image;
}

const QImage *LytroImage::getQImage() const
{
	return m_image;
}
