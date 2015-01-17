#include "lytroimage.h"

#include <QtGui/QImage>

#include <cstddef>
#include <cstdint>
#include <fstream>

namespace {
const std::size_t IMG_WIDTH=3280;
const std::size_t IMG_HEIGHT=3280;

uint8_t bilinearInterpolation(double f00, double f01, double f10, double f11) {
	double a = 0.5 * (f00 + f10);
	double b = 0.5 * (f01 + f11);
	return 0.5 * (a + b);
}

uint8_t avgInterpolation(double f00, double f01, double f10, double f11) {
	return 0.25 * (f00 + f01 + f10 + f11);
}

}

LytroImage::LytroImage(const char *file)
{
	m_image = new QImage(IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32);
	auto imageArray = new uint16_t[IMG_WIDTH][IMG_HEIGHT];
	
	std::fstream fin(file, std::fstream::in | std::fstream::binary);
	
	std::size_t pos(0);
	char buf[4];
	std::uint16_t tmp12b;
	std::size_t x, y;
	while (true) {
		if (pos / IMG_WIDTH >= IMG_HEIGHT) {
			break;
		}
		
		fin.read(buf, 3);
		
		// mask out the first and twelve bits
		// the data are stored as big endian, so we have to fix that, too
		tmp12b = ((buf[1] & 0xF0) << 8) | buf[0];
		y = pos / IMG_WIDTH;
		x = pos % IMG_WIDTH;
		imageArray[x][y] = tmp12b;
		++pos;
		
		// the second 12b
		tmp12b = ((buf[2] & 0xF) << 12) | ((buf[1] & 0xF) << 4) | ((buf[2] >> 4) & 0xF);
		y = pos / IMG_WIDTH;
		x = pos % IMG_WIDTH;
		imageArray[x][y] = tmp12b;
		++pos;
	}
	fin.close();
	
	// separate the colors
	auto imageArraySeparated = new uint16_t[IMG_WIDTH][IMG_HEIGHT][3];
	for (y = 0; y < IMG_HEIGHT; ++y) {
		for (x = 0; x < IMG_WIDTH; ++x) {
			const uint8_t point(imageArray[x][y]);
			
			if ((y & 1) != 0) {
				// even lines - red/green
				if ((x & 1) != 0) {
					imageArraySeparated[x][y][0] = point;
				}
				else {
					imageArraySeparated[x][y][1] = point;
				}
			}
			else {
				// odd lines - blue/green
				if ((x & 1) != 0) {
					imageArraySeparated[x][y][1] = point;
				}
				else {
					imageArraySeparated[x][y][2] = point;
				}
			}
		}
	}
	
	// interpolate the colors
	// red lines and blue stripes
	for (y = 1; y < IMG_HEIGHT; y += 2) {
		for (x = 2; x < IMG_WIDTH; x += 2) {
			// red line
			imageArraySeparated[x][y][0] = 0.5 * (imageArraySeparated[x-1][y][0]+ imageArraySeparated[x+1][y][0]);
			// blue stripe
			imageArraySeparated[x][y][2] = 0.5 * (imageArraySeparated[x][y-1][2] + imageArraySeparated[x][y+1][2]);
		}
	}
	// blue lines and red stripes
	for (y = 2; y < IMG_HEIGHT; y += 2) {
		for (x = 1; x < IMG_WIDTH; x += 2) {
			// red stripe
			imageArraySeparated[x][y][0] = 0.5 * (imageArraySeparated[x][y-1][0] + imageArraySeparated[x][y+1][0]);
			// blue line
			imageArraySeparated[x][y][2] = 0.5 * (imageArraySeparated[x-1][y][2] + imageArraySeparated[x+1][y][2]);
		}
	}
	// red middle, green #1
	for (y = 2; y < IMG_HEIGHT; y += 2) {
		for (x = 2; x < IMG_WIDTH; x += 2) {
			imageArraySeparated[x][y][0] = bilinearInterpolation(imageArraySeparated[x-1][y+1][0], imageArraySeparated[x-1][y-1][0],
			                                            imageArraySeparated[x+1][y+1][0], imageArraySeparated[x+1][y-1][0]);
			imageArraySeparated[x][y][1] = avgInterpolation(imageArraySeparated[x-1][y][1], imageArraySeparated[x+1][y][1],
			                                              imageArraySeparated[x][y-1][1], imageArraySeparated[x][y+1][1]);
		}
	}
	// blue middle, green #2
	for (y = 1; y < IMG_HEIGHT; y += 2) {
		for (x = 1; x < IMG_WIDTH; x += 2) {
			imageArraySeparated[x][y][2] = bilinearInterpolation(imageArraySeparated[x-1][y+1][2], imageArraySeparated[x-1][y-1][2],
			                                             imageArraySeparated[x+1][y+1][2], imageArraySeparated[x+1][y-1][2]);
			imageArraySeparated[x][y][1] = avgInterpolation(imageArraySeparated[x-1][y][1], imageArraySeparated[x+1][y][1],
			                                              imageArraySeparated[x][y-1][1], imageArraySeparated[x][y+1][1]);
		}
	}
	
	// combine the images
	for(y = 0; y < IMG_HEIGHT; ++y) {
		for(x = 0; x < IMG_WIDTH; ++x) {
			m_image->setPixel(x, y, qRgb(imageArraySeparated[x][y][0], imageArraySeparated[x][y][1], imageArraySeparated[x][y][2]));
		}
	}
}

LytroImage::~LytroImage()
{

}

const QImage *LytroImage::getQImage() const
{
	return m_image;
}
