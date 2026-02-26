#include "kirtform.h"
#include "ui_kirtform.h"

KirtForm::KirtForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::KirtForm)
{
    ui->setupUi(this);
}

KirtForm::~KirtForm()
{
    delete ui;
}
