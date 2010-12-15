#ifndef ADC_FORM_H
#define ADC_FORM_H

#include <QWidget>
#include "module_adc.h"

namespace Ui {
    class ADC_Form;
}

class ADC_Form : public QWidget
{
    Q_OBJECT

public:
    explicit ADC_Form(Module_ADC *module, QWidget *parent = 0);
    ~ADC_Form();

private:
    Ui::ADC_Form *ui;
    Module_ADC *module;

private slots:
    void on_save_clicked();
};

#endif // ADC_FORM_H
