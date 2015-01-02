#ifndef THUMBNAILPROVIDER_H
#define THUMBNAILPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>

#include <camera.h>

class ThumbnailProvider : public QObject, public QQuickImageProvider
{
	Q_OBJECT
	
public:
	ThumbnailProvider();
	~ThumbnailProvider();
	
    virtual QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override;

public slots:
	void changeCamera(Lyli::Camera *camera);

private:
	Lyli::Camera *m_camera;
};

#endif // THUMBNAILPROVIDER_H
