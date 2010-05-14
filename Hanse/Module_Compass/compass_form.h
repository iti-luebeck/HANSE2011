#ifndef COMPASS_FORM_H
#define COMPASS_FORM_H

#include <QWidget>
#include <Module_Compass/module_compass.h>

namespace Ui {
    class Compass_Form;
}

class Compass_Form : public QWidget {
    Q_OBJECT
public:
    Compass_Form(Module_Compass *module, QWidget *parent = 0);
    ~Compass_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Compass_Form *ui;
    Module_Compass *module;

private slots:
    void on_calibStop_clicked();
    void on_calibStart_clicked();
    void on_save_clicked();
    void dataChanged(RobotModule *module);
};

#endif // COMPASS_FORM_H
