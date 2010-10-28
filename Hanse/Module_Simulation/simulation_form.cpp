#include "simulation_form.h"
#include "ui_simulation_form.h"

Simulation_Form::Simulation_Form(Module_Simulation *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Simulation_Form)
{
    ui->setupUi(this);
    this->module = module;
    ui->server_ip_adress->setText(module->getSettingsValue("server_ip_adress").toString());
    ui->server_port->setText(module->getSettingsValue("server_port").toString());
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
    module->setSettingsValue("server_ip_adress", ui->server_ip_adress->text());
    module->setSettingsValue("server_port", ui->server_port->text().toInt());
}
