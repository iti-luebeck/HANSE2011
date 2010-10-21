#include "pressure_form.h"
#include "ui_pressure_form.h"

Pressure_Form::Pressure_Form(Module_PressureSensor *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Pressure_Form)
{
    ui->setupUi(this);
    this->module = module;
    ui->i2cAddress->setText(module->getSettingsValue("i2cAddress").toString());
    ui->frequency->setText(module->getSettingsValue("frequency").toString());
    ui->airPressure->setText(module->getSettingsValue("airPressure").toString());
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
    module->setSettingsValue("i2cAddress", ui->i2cAddress->text().toInt());
    module->setSettingsValue("airPressure", ui->airPressure->text().toInt());
    module->setSettingsValue("frequency", ui->frequency->text().toInt());
}
