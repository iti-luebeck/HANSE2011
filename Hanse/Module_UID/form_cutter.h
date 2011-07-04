#ifndef FORM_H
#define FORM_H

#include "Module_Cutter.h"
#include <QWidget>

namespace Ui {
    class Form_Cutter;
}

class FormCutter : public QWidget {
    Q_OBJECT
public:
    FormCutter(Module_Cutter* module, QWidget *parent = 0);
    ~FormCutter();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Cutter *ui;
    Module_Cutter *module;

private slots:
    void on_scan_clicked();
    void on_save_clicked();
};

#endif // FORM_H
