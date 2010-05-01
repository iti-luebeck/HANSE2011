#ifndef THRUSTER_FORM_H
#define THRUSTER_FORM_H

#include <QWidget>
#include "module_thruster.h"

namespace Ui {
    class Thruster_Form;
}

class Thruster_Form : public QWidget {
    Q_OBJECT
public:
    Thruster_Form(Module_Thruster *module, QWidget *parent = 0);
    ~Thruster_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Thruster_Form *ui;
    Module_Thruster *thruster;

private slots:
    void on_save_clicked();
};

#endif // THRUSTER_FORM_H
