#ifndef PRESSURE_FORM_H
#define PRESSURE_FORM_H

#include <QWidget>
#include "module_pressuresensor.h"

namespace Ui {
    class Pressure_Form;
}

class Pressure_Form : public QWidget {
    Q_OBJECT
public:
    Pressure_Form(Module_PressureSensor *module, QWidget *parent = 0);
    ~Pressure_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Pressure_Form *ui;
};

#endif // PRESSURE_FORM_H
