#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/qmainwindow.h>

#include "cameraform.h"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();

private:
    Ui::MainWindow* ui;
	
	CameraForm *m_cameraForm;
};

#endif // MAINWINDOW_H
