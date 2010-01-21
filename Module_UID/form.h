#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include "module_uid.h"

namespace Ui {
    class Form;
}

class Form : public QWidget {
    Q_OBJECT
public:
    Form(Module_UID* module, QWidget *parent = 0);
    ~Form();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form *ui;
    Module_UID *module;

private slots:
    void on_save_clicked();
};

#endif // FORM_H
