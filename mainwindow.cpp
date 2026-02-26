#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("Kirt - MPos Log Viewer ver: " + QString(APP_VERSION_STR) + " " + QString(APP_BUILD_DATETIME));

}

MainWindow::~MainWindow()
{
    delete ui;
}
