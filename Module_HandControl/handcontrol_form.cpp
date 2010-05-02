#include "handcontrol_form.h"
#include "ui_handcontrol_form.h"

HandControl_Form::HandControl_Form(Module_HandControl *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HandControl_Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->port->setText(module->getSettings().value("port").toString());
    ui->divisor->setText(module->getSettings().value("divisor").toString());
    if (module->getSettings().value("receiver").toString()=="thruster") {
        ui->controlThruster->setChecked(true);
    } else {
        ui->controlTCL->setChecked(true);
    }

    connect(module->server, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()));
}

HandControl_Form::~HandControl_Form()
{
    delete ui;
}

void HandControl_Form::changeEvent(QEvent *e)
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

void HandControl_Form::on_save_clicked()
{
    module->getSettings().setValue("port", ui->port->text().toInt());
    module->getSettings().setValue("divisor", ui->divisor->text().toFloat());

    if (ui->controlThruster->isChecked())
        module->getSettings().setValue("receiver","thruster");
    else
        module->getSettings().setValue("receiver","controlLoop");

    module->reset();

}

void HandControl_Form::connectionStatusChanged()
{
    if (module->server->isConnected()) {
        ui->connectionStatus->setText("Connected.");
    } else {
        ui->connectionStatus->setText("Not connected.");
    }
}

