#ifndef FORM_H
#define FORM_H

#include "module_uid.h"
#include <QWidget>

namespace Ui {
    class Form_UID;
}

class FormUID : public QWidget {
    Q_OBJECT
public:
    FormUID(Module_UID* module, QWidget *parent = 0);
    ~FormUID();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_UID *ui;
    Module_UID *module;

private slots:
    void on_scan_clicked();
    void on_save_clicked();
};

#endif // FORM_H
