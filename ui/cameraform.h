#ifndef CAMERAFORM_H
#define CAMERAFORM_H

#include <QtWidgets/QWidget>

namespace Ui
{
class CameraForm;
}
class QModelIndex;

class CameraForm : public QWidget
{
	Q_OBJECT
public:
	CameraForm(QWidget *parent = nullptr);
	~CameraForm();

private slots:
	void changeCamera(const QModelIndex& index);
	
private:
	Ui::CameraForm* ui;
	
	CameraForm(const CameraForm& other);
	CameraForm& operator=(const CameraForm& other);
};

#endif // CAMERAFORM_H
