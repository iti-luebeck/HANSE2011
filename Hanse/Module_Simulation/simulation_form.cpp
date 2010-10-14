#include "simulation_form.h"
#include "ui_simulation_form.h"

Simulation_Form::Simulation_Form(Module_Simulation *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Simulation_Form)
{
    ui->setupUi(this);
    this->module = module;
    ui->i2cAddress->setText(module->getSettings().value("i2cAddress").toString());
    ui->frequency->setText(module->getSettings().value("frequency").toString());
    ui->airPressure->setText(module->getSettings().value("airPressure").toString());
}

Simulation_Form::~Simulation_Form()
{
    delete ui;
}

void Simulation_Form::changeEvent(QEvent *e)
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

void Simulation_Form::on_save_clicked()
{
    module->getSettings().setValue("i2cAddress", ui->i2cAddress->text().toInt());
    module->getSettings().setValue("airPressure", ui->airPressure->text().toInt());
    module->getSettings().setValue("frequency", ui->frequency->text().toInt());
}
