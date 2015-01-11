#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow()
{
	ui = new Ui::MainWindow;
	ui->setupUi(this);
	
	QHBoxLayout *cameraFormLayout = new QHBoxLayout;
	cameraFormLayout->addWidget(new CameraForm(ui->tabCamera));
	
	ui->tabCamera->setLayout(cameraFormLayout);
}

MainWindow::~MainWindow()
{
	delete ui;
}
