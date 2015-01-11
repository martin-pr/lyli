#include "cameraform.h"
#include "ui_cameraform.h"

#include "cameralistmodel.h"
#include "imagelistdelegate.h"
#include "imagelistmodel.h"

CameraForm::CameraForm(QWidget *parent) : QWidget(parent)
{
	ui = new Ui::CameraForm;
	ui->setupUi(this);
	
	// set the models
	ui->cameraList->setModel(new CameraListModel);
	ui->imageList->setModel(new ImageListModel);
	
	// set the delegates
	ui->imageList->setItemDelegate(new ImageListDelegate);
	
	// connect signals & slots
	connect(ui->cameraList, SIGNAL(activated(QModelIndex)), this, SLOT(changeCamera(QModelIndex)));
}

CameraForm::~CameraForm()
{
	delete ui;
}

void CameraForm::changeCamera(const QModelIndex &index)
{
	CameraListModel *model = static_cast<CameraListModel*>(ui->cameraList->model());
	Lyli::Camera *camera = model->getCamera(index.row());
	if (camera != nullptr) {
		ImageListModel *imageModel = static_cast<ImageListModel*>(ui->imageList->model());
		imageModel->changeCamera(camera);
	}
}
