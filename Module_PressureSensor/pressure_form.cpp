#include "pressure_form.h"
#include "ui_pressure_form.h"

Pressure_Form::Pressure_Form(Module_PressureSensor *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pressure_Form)
{
    ui->setupUi(this);
    this->module = module;
    ui->i2cAddress->setText(module->getSettings().value("i2cAddress").toString());
    ui->frequency->setText(module->getSettings().value("frequency").toString());
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

void Pressure_Form::on_save_clicked()
{
    module->getSettings().setValue("i2cAddress", ui->i2cAddress->text().toInt(0,0));
    module->getSettings().setValue("frequency", ui->frequency->text().toInt(0,0));
}
