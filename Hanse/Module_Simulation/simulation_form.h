#ifndef SIMULATION_FORM_H
#define SIMULATION_FORM_H

#include <QWidget>
#include "module_simulation.h"

namespace Ui {
    class Simulation_Form;
}

class Simulation_Form : public QWidget {
    Q_OBJECT
public:
    Simulation_Form(Module_Simulation *module, QWidget *parent = 0);
    ~Simulation_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Simulation_Form *ui;
    Module_Simulation *module;

private slots:
    void on_save_clicked();
};

#endif // SIMULATION_FORM_H
