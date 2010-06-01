#include "handcontrol_form.h"
#include "ui_handcontrol_form.h"

HandControl_Form::HandControl_Form(Module_HandControl *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HandControl_Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->port->setText(module->getSettings().value("port").toString());
    ui->divFw->setText(module->getSettings().value("divFw").toString());
    ui->divLR->setText(module->getSettings().value("divLR").toString());
    ui->divUD->setText(module->getSettings().value("divUD").toString());
    if (module->getSettings().value("receiver").toString()=="thruster") {
        ui->controlThruster->setChecked(true);
    } else {
        ui->controlTCL->setChecked(true);
    }

    ui->sliderFw->setMaximum(ui->divFw->text().toInt());
    ui->sliderLR->setMaximum(ui->divLR->text().toInt());
    ui->sliderUD->setMaximum(ui->divUD->text().toInt());
    ui->sliderFw->setMinimum(-ui->divFw->text().toInt());
    ui->sliderLR->setMinimum(-ui->divLR->text().toInt());
    ui->sliderUD->setMinimum(-ui->divUD->text().toInt());

    connect(module->server, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()));
    connect(module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(dataChanged(RobotModule*)));
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
    module->getSettings().setValue("divFw", ui->divFw->text().toFloat());
    module->getSettings().setValue("divUD", ui->divUD->text().toFloat());
    module->getSettings().setValue("divLR", ui->divLR->text().toFloat());

    if (ui->controlThruster->isChecked())
        module->getSettings().setValue("receiver","thruster");
    else
        module->getSettings().setValue("receiver","controlLoop");

    module->getSettings().setValue("enableGamepad", ui->enableGamepad->isChecked());

    ui->sliderFw->setMaximum(ui->divFw->text().toInt());
    ui->sliderLR->setMaximum(ui->divLR->text().toInt());
    ui->sliderUD->setMaximum(ui->divUD->text().toInt());
    ui->sliderFw->setMinimum(-ui->divFw->text().toInt());
    ui->sliderLR->setMinimum(-ui->divLR->text().toInt());
    ui->sliderUD->setMinimum(-ui->divUD->text().toInt());

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

void HandControl_Form::dataChanged(RobotModule *m)
{

    int forwardSpeed = m->getData().value("forwardSpeed").toInt();
    int angularSpeed = m->getData().value("angularSpeed").toInt();
    int speedUpDown = m->getData().value("speedUpDown").toInt();

    ui->sliderFw->setValue(forwardSpeed);
    ui->sliderLR->setValue(angularSpeed);
    ui->sliderUD->setValue(speedUpDown);
}

void HandControl_Form::on_sliderFw_valueChanged(int value)
{
    module->data["forwardSpeed"] = value;
    module->sendNewControls();
}


void HandControl_Form::on_sliderLR_valueChanged(int value)
{
    module->data["angularSpeed"] = value;
    module->sendNewControls();
}

void HandControl_Form::on_sliderUD_valueChanged(int value)
{
    module->data["speedUpDown"] = value;
    module->sendNewControls();
}
