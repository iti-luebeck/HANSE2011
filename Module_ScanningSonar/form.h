#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "module_scanningsonar.h"

namespace Ui {
    class Form;
}

class Form : public QWidget {
    Q_OBJECT
public:
    Form(Module_ScanningSonar* sonar, QWidget *parent = 0);
    ~Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form *ui;
    Module_ScanningSonar* sonar;

private slots:
    void on_pushButton_clicked();
};

#endif // FORM_H
