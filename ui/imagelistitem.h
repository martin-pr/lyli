#ifndef IMAGELISTITEM_H
#define IMAGELISTITEM_H

#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QString>
#include <QtGui/QImage>

#include <camera.h>

class ImageListItem : public QObject
{
	Q_OBJECT

public:
    ImageListItem();
	ImageListItem(Lyli::Camera *camera, const Lyli::FileListEntry &fileEntry, QObject* parent = nullptr);
	ImageListItem(const ImageListItem& other);
	~ImageListItem();
	
	bool isNull() const;
	
	int getId() const;
	QString getSha1() const;
	QDateTime getTime() const;
	QImage getImage() const;
	
	Q_PROPERTY(bool null READ isNull)
	Q_PROPERTY(int id READ getId)
	Q_PROPERTY(QString sha1 READ getSha1)
	Q_PROPERTY(QDateTime time READ getTime)
	Q_PROPERTY(QImage image READ getImage)

private:
	Lyli::Camera *m_camera;
	Lyli::FileListEntry m_fileEntry;
	// cached image
	mutable QImage m_image;
};

Q_DECLARE_METATYPE(ImageListItem)

#endif // IMAGELISTITEM_H
