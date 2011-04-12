#ifndef XSENS_FORM_H
#define XSENS_FORM_H

#include <QWidget>

class Module_XsensMTi;

namespace Ui {
    class Xsens_Form;
}

class Xsens_Form : public QWidget
{
    Q_OBJECT

public:
    explicit Xsens_Form(Module_XsensMTi *mti, QWidget *parent = 0);
    ~Xsens_Form();

private:
    Ui::Xsens_Form *ui;
    Module_XsensMTi *mti;

private slots:
    void on_pushButton_clicked();
};

#endif // XSENS_FORM_H
