#ifndef KIRTFORM_H
#define KIRTFORM_H

#include <QWidget>

namespace Ui {
class KirtForm;
}

class KirtForm : public QWidget
{
    Q_OBJECT

public:
    explicit KirtForm(QWidget *parent = nullptr);
    ~KirtForm();

private:
    Ui::KirtForm *ui;
};

#endif // KIRTFORM_H
