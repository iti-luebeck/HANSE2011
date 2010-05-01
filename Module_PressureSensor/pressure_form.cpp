#include "pressure_form.h"
#include "ui_pressure_form.h"

Pressure_Form::Pressure_Form(Module_PressureSensor *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pressure_Form)
{
    ui->setupUi(this);
}

Pressure_Form::~Pressure_Form()
{
    delete ui;
}

void Pressure_Form::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
