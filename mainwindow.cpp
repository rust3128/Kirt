#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "version.h"
#include "Kirt/kirtform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createUI()
{
    this->setWindowTitle("Kirt - MPos Log Viewer ver: " + QString(APP_VERSION_STR) + " " + QString(APP_BUILD_DATETIME));

    auto *kirtForm = new KirtForm(this);
    this->setCentralWidget(kirtForm);
}
