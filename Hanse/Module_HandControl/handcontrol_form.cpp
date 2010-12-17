#include "handcontrol_form.h"
#include "ui_handcontrol_form.h"

HandControl_Form::HandControl_Form(Module_HandControl *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HandControl_Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->port->setText(module->getSettingsValue("port").toString());
    ui->divFw->setText(module->getSettingsValue("divFw").toString());
    ui->divLR->setText(module->getSettingsValue("divLR").toString());
    ui->divUD->setText(module->getSettingsValue("divUD").toString());
    if (module->getSettingsValue("receiver").toString()=="thruster") {
        ui->controlThruster->setChecked(true);
    } else {
        ui->controlTCL->setChecked(true);
    }

    ui->sliderFw->setMaximum(ui->divFw->text().toInt());
    ui->sliderLR->setMaximum(ui->divLR->text().toInt());
    ui->sliderFw->setMinimum(-ui->divFw->text().toInt());
    ui->sliderLR->setMinimum(-ui->divLR->text().toInt());

//    connect(module->server, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()));
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
    module->setSettingsValue("port", ui->port->text().toInt());
    module->setSettingsValue("divFw", ui->divFw->text().toFloat());
    module->setSettingsValue("divUD", ui->divUD->text().toFloat());
    module->setSettingsValue("divLR", ui->divLR->text().toFloat());

    if (ui->controlThruster->isChecked())
        module->setSettingsValue("receiver","thruster");
    else
        module->setSettingsValue("receiver","controlLoop");

    module->setSettingsValue("enableGamepad", ui->enableGamepad->isChecked());

    ui->sliderFw->setMaximum(ui->divFw->text().toInt());
    ui->sliderLR->setMaximum(ui->divLR->text().toInt());
    ui->sliderUD->setMaximum(127);
    ui->sliderFw->setMinimum(-ui->divFw->text().toInt());
    ui->sliderLR->setMinimum(-ui->divLR->text().toInt());
    ui->sliderUD->setMinimum(-127);

    module->reset();

}

void HandControl_Form::connectionStatusChanged()
{
//    if (module->server->isConnected()) {
//        ui->connectionStatus->setText("Connected.");
//    } else {
//        ui->connectionStatus->setText("Not connected.");
//    }
}

void HandControl_Form::dataChanged(RobotModule *m)
{

    int forwardSpeed = m->getDataValue("forwardSpeed").toInt();
    int angularSpeed = m->getDataValue("angularSpeed").toInt();
    int speedUpDown = m->getDataValue("speedUpDown").toInt();

    ui->sliderFw->setValue(forwardSpeed);
    ui->sliderLR->setValue(angularSpeed);
    ui->sliderUD->setValue(speedUpDown);
}

void HandControl_Form::on_sliderFw_valueChanged(int value)
{
    module->addData("forwardSpeed", value);
    module->sendNewControls();
}


void HandControl_Form::on_sliderLR_valueChanged(int value)
{
    module->addData("angularSpeed", value);
    module->sendNewControls();
}

void HandControl_Form::on_sliderUD_valueChanged(int value)
{
    module->addData("speedUpDown", value);
    module->sendNewControls();
}
