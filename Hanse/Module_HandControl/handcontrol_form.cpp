#include "handcontrol_form.h"
#include "ui_handcontrol_form.h"

HandControl_Form::HandControl_Form(Module_HandControl *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HandControl_Form)
{
    ui->setupUi(this);
    this->module = module;

    maxForwardSpeed = module->getSettingsValue("maxForSpeed").toFloat();
    maxAngularSpeed = module->getSettingsValue("maxAngSpeed").toFloat();
    maxUpDownSpeed = module->getSettingsValue("maxVertSpeed").toFloat();

    ui->port->setText(module->getSettingsValue("port").toString());
    ui->divFw->setText(module->getSettingsValue("divFw").toString());
    ui->divLR->setText(module->getSettingsValue("divLR").toString());
    ui->divUD->setText(module->getSettingsValue("divUD").toString());
    ui->maxForSpeed->setText(module->getSettingsValue("maxForSpeed").toString());
    ui->maxAngSpeed->setText(module->getSettingsValue("maxAngSpeed").toString());
    ui->maxVertSpeed->setText(module->getSettingsValue("maxVertSpeed").toString());

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
    connect(this,SIGNAL(updateControls()),module,SLOT(sendNewControls()));


    forward = new QAction(this);
    forward->setShortcut(Qt::Key_W);
    backward = new QAction(this);
    backward->setShortcut(Qt::Key_S);
    left = new QAction(this);
    left->setShortcut(Qt::Key_A);
    right = new QAction(this);
    right->setShortcut(Qt::Key_D);
    up = new QAction(this);
    up->setShortcut(Qt::Key_U);
    down = new QAction(this);
    down->setShortcut(Qt::Key_J);

    connect(forward,SIGNAL(triggered()),this,SLOT(forwardPressed()));
    connect(backward, SIGNAL(triggered()),this,SLOT(backwardPressed()));
    connect(left, SIGNAL(triggered()),this,SLOT(leftPressed()));
    connect(right, SIGNAL(triggered()),this,SLOT(rightPressed()));
    connect(up, SIGNAL(triggered()),this,SLOT(upPressed()));
    connect(down, SIGNAL(triggered()),this,SLOT(downPressed()));

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
    module->setSettingsValue("maxForSpeed",ui->maxForSpeed->text().toFloat());
    module->setSettingsValue("maxAngSpeed", ui->maxAngSpeed->text().toFloat());
    module->setSettingsValue("maxVertSpeed", ui->maxVertSpeed->text().toFloat());

    maxForwardSpeed = ui->maxForSpeed->text().toFloat();
    maxAngularSpeed = ui->maxAngSpeed->text().toFloat();
    maxUpDownSpeed = ui->maxVertSpeed->text().toFloat();

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
//    module->sendNewControls();
    emit updateControls();
}


void HandControl_Form::on_sliderLR_valueChanged(int value)
{
    module->addData("angularSpeed", value);
    emit updateControls();
//    module->sendNewControls();
}

void HandControl_Form::on_sliderUD_valueChanged(int value)
{
    module->addData("speedUpDown", value);
    emit updateControls();
//    module->sendNewControls();
}

void HandControl_Form::forwardPressed(){
    qDebug()<<"Vorwaerts";
    module->addData("forwardSpeed", maxForwardSpeed);
    emit updateControls();
}

void HandControl_Form::backwardPressed(){
    qDebug()<<"Rueckwaerts";
    module->addData("forwardSpeed", -maxForwardSpeed);
    emit updateControls();
}

void HandControl_Form::leftPressed(){
    qDebug()<<"Links";
    module->addData("angularSpeed", maxAngularSpeed);
    emit updateControls();
}

void HandControl_Form::rightPressed(){
    qDebug()<<"Rechts";
    module->addData("angularSpeed", -maxAngularSpeed);
    emit updateControls();
}

void HandControl_Form::upPressed(){
    qDebug()<<"Hoch";
    module->addData("speedUpDown", maxUpDownSpeed);
    emit updateControls();
}

void HandControl_Form::downPressed(){
    qDebug()<<"Runter";
    module->addData("speedUpDown", -maxUpDownSpeed);
    emit updateControls();
}
