#ifndef LYTROIMAGE_H
#define LYTROIMAGE_H

class QImage;

class LytroImage
{
public:
	LytroImage(const char *file);
	~LytroImage();
	
	const QImage *getQImage() const;
	
private:
	QImage *m_image;
};

#endif // LYTROIMAGE_H
