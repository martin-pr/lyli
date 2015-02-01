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



#ifndef LYTROIMAGE_H
#define LYTROIMAGE_H

class QImage;

class LytroImage
{
public:
	LytroImage();
	LytroImage(const char *file);
	~LytroImage();
	
	LytroImage(const LytroImage &other);
	LytroImage(LytroImage &&other);
	LytroImage &operator=(const LytroImage &other);
	LytroImage &operator=(LytroImage &&other);
	
	/** Initialize static members.
	 * 
	 */
	static void init();
	
	const QImage *getQImage() const;
	
private:
	QImage *m_image;
	static unsigned char m_gamma[4096];
};

#endif // LYTROIMAGE_H
