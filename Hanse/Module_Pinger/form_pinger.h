#ifndef FORM_H
#define FORM_H

#include <Module_Pinger/module_pinger.h>
#include <QWidget>

namespace Ui {
    class Form_Pinger;
}

class FormPinger : public QWidget {
    Q_OBJECT
public:
    FormPinger(Module_Pinger* module, QWidget *parent = 0);
    ~FormPinger();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Pinger *ui;
    Module_Pinger *module;

private slots:
    void on_save_clicked();
};

#endif // FORM_H
