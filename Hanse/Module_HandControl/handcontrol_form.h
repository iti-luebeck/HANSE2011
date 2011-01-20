#ifndef HANDCONTROL_FORM_H
#define HANDCONTROL_FORM_H

#include <QWidget>
#include "module_handcontrol.h"
#include "server.h"

namespace Ui {
    class HandControl_Form;
}

class HandControl_Form : public QWidget {
    Q_OBJECT
public:
    HandControl_Form(Module_HandControl *module, QWidget *parent = 0);
    ~HandControl_Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::HandControl_Form *ui;
    Module_HandControl *module;

private slots:
    void on_sliderUD_valueChanged(int value);
    void on_sliderLR_valueChanged(int value);
    void on_sliderFw_valueChanged(int value);
    void on_save_clicked();
    void connectionStatusChanged();
    void dataChanged(RobotModule* m);

signals:
    void updateControls();
};

#endif // HANDCONTROL_FORM_H
