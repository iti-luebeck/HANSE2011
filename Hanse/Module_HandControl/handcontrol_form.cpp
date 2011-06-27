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
    stepsize = module->getSettingsValue("maxVertSpeed").toFloat();
    upDownSpeed = 0.0;


    ui->port->setText(module->getSettingsValue("port").toString());
    ui->divFw->setText(module->getSettingsValue("divFw").toString());
    ui->divLR->setText(module->getSettingsValue("divLR").toString());
    ui->divUD->setText(module->getSettingsValue("divUD").toString());
    ui->maxForSpeed->setText(module->getSettingsValue("maxForSpeed").toString());
    ui->maxAngSpeed->setText(module->getSettingsValue("maxAngSpeed").toString());
    ui->maxVertSpeed->setText(module->getSettingsValue("maxVertSpeed").toString());
    ui->resetTime->setText(module->getSettingsValue("resetTime").toString());
    ui->enableGamepad->setChecked(module->getSettingsValue("enableGamepad").toBool());

    if (module->getSettingsValue("receiver").toString()=="thruster") {
        ui->controlThruster->setChecked(true);
    } else {
        ui->controlTCL->setChecked(true);
    }

    //    connect(module->server, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()));
    //    connect(module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(dataChanged(RobotModule*)));
    connect(this,SIGNAL(updateControls()),module,SLOT(sendNewControls()));
    connect(&forTimer,SIGNAL(timeout()),this,SLOT(resetForSpeeds()));
    connect(&angTimer,SIGNAL(timeout()),this,SLOT(resetAngSpeeds()));
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
    module->setSettingsValue("resetTime", ui->resetTime->text().toInt());

    maxForwardSpeed = ui->maxForSpeed->text().toFloat();
    maxAngularSpeed = ui->maxAngSpeed->text().toFloat();
    stepsize = ui->maxVertSpeed->text().toFloat();

    if (ui->controlThruster->isChecked())
        module->setSettingsValue("receiver","thruster");
    else
        module->setSettingsValue("receiver","controlLoop");

    module->setSettingsValue("enableGamepad", ui->enableGamepad->isChecked());

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

//void HandControl_Form::dataChanged(RobotModule *m)
//{

//    int forwardSpeed = m->getDataValue("forwardSpeed").toInt();
//    int angularSpeed = m->getDataValue("angularSpeed").toInt();
//    int speedUpDown = m->getDataValue("speedUpDown").toInt();

//    ui->sliderFw->setValue(forwardSpeed);
//    ui->sliderLR->setValue(angularSpeed);
//    ui->sliderUD->setValue(speedUpDown);
//}

void HandControl_Form::on_forwardButton_clicked()
{
    forTimer.start(resetTime);
    module->addData("forwardSpeed", maxForwardSpeed);
    emit updateControls();
}

void HandControl_Form::on_leftButton_clicked()
{
    angTimer.start(resetTime);
    module->addData("angularSpeed", -maxAngularSpeed);
    emit updateControls();
}

void HandControl_Form::on_backwardButton_clicked()
{
    forTimer.start(resetTime);
    module->addData("forwardSpeed", -maxForwardSpeed);
    emit updateControls();
}

void HandControl_Form::on_rightButton_clicked()
{
    angTimer.start(resetTime);
    module->addData("angularSpeed", maxAngularSpeed);
    emit updateControls();
}

void HandControl_Form::on_upButton_clicked()
{
    upDownSpeed = upDownSpeed - stepsize;
    if(upDownSpeed < 0.0){
        upDownSpeed = 0.0;
    }
    module->addData("speedUpDown", upDownSpeed);
    emit updateControls();
}

void HandControl_Form::on_downButton_clicked()
{
    upDownSpeed = upDownSpeed + stepsize;
    module->addData("speedUpDown", upDownSpeed);
    emit updateControls();
}

void HandControl_Form::resetForSpeeds(){
    module->addData("forwardSpeed", 0);
    emit updateControls();
    forTimer.stop();
}

void HandControl_Form::resetAngSpeeds(){
    module->addData("angularSpeed", 0);
    emit updateControls();
    angTimer.stop();
}

void HandControl_Form::on_resetDepthButton_clicked()
{
    upDownSpeed = 0.0;
    module->addData("speedUpDown", upDownSpeed);
    emit updateControls();
}



//#include "handcontrol_form.h"
//#include "ui_handcontrol_form.h"

//HandControl_Form::HandControl_Form(Module_HandControl *module, QWidget *parent) :
//        QWidget(parent),
//        ui(new Ui::HandControl_Form)
//{
//    ui->setupUi(this);
//    this->module = module;

//    maxForwardSpeed = module->getSettingsValue("maxForSpeed").toFloat();
//    maxAngularSpeed = module->getSettingsValue("maxAngSpeed").toFloat();
//    stepsize = module->getSettingsValue("maxVertSpeed").toFloat();
//    resetTime = module->getSettingsValue("resetTime").toInt();
//    upDownSpeed = 0.0;


//    ui->port->setText(module->getSettingsValue("port").toString());
//    ui->divFw->setText(module->getSettingsValue("divFw").toString());
//    ui->divLR->setText(module->getSettingsValue("divLR").toString());
//    ui->divUD->setText(module->getSettingsValue("divUD").toString());
//    ui->maxForSpeed->setText(module->getSettingsValue("maxForSpeed").toString());
//    ui->maxAngSpeed->setText(module->getSettingsValue("maxAngSpeed").toString());
//    ui->maxVertSpeed->setText(module->getSettingsValue("maxVertSpeed").toString());
//    ui->resetTime->setText(module->getSettingsValue("resetTime").toString());

//    if (module->getSettingsValue("receiver").toString()=="thruster") {
//        ui->controlThruster->setChecked(true);
//    } else {
//        ui->controlTCL->setChecked(true);
//    }

//    //    connect(module->server, SIGNAL(statusChanged()), this, SLOT(connectionStatusChanged()));
//    //    connect(module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(dataChanged(RobotModule*)));
//    connect(this,SIGNAL(updateControls(int, int, int)),module,SLOT(newMessage(int, int, int)));
////    connect(&forTimer,SIGNAL(timeout()),this,SLOT(resetForSpeeds()));
////    connect(&angTimer,SIGNAL(timeout()),this,SLOT(resetAngSpeeds()));

//    connect(&speedTimer, SIGNAL(timeout()), this, SLOT(setSpeeds()));
//    speedTimer.start(resetTime);
//}

//HandControl_Form::~HandControl_Form()
//{
//    delete ui;
//}

//void HandControl_Form::changeEvent(QEvent *e)
//{
//    QWidget::changeEvent(e);
//    switch (e->type()) {
//    case QEvent::LanguageChange:
//        ui->retranslateUi(this);
//        break;
//    default:
//        break;
//    }
//}

//void HandControl_Form::on_save_clicked()
//{
//    module->setSettingsValue("port", ui->port->text().toInt());
//    module->setSettingsValue("divFw", ui->divFw->text().toFloat());
//    module->setSettingsValue("divUD", ui->divUD->text().toFloat());
//    module->setSettingsValue("divLR", ui->divLR->text().toFloat());
//    module->setSettingsValue("maxForSpeed",ui->maxForSpeed->text().toFloat());
//    module->setSettingsValue("maxAngSpeed", ui->maxAngSpeed->text().toFloat());
//    module->setSettingsValue("maxVertSpeed", ui->maxVertSpeed->text().toFloat());
//    module->setSettingsValue("resetTime", ui->resetTime->text().toInt());

//    resetTime = module->getSettingsValue("resetTime").toInt();
//    maxForwardSpeed = ui->maxForSpeed->text().toFloat();
//    maxAngularSpeed = ui->maxAngSpeed->text().toFloat();
//    stepsize = ui->maxVertSpeed->text().toFloat();

//    if (ui->controlThruster->isChecked())
//        module->setSettingsValue("receiver","thruster");
//    else
//        module->setSettingsValue("receiver","controlLoop");

//    module->setSettingsValue("enableGamepad", ui->enableGamepad->isChecked());

//    module->reset();

//}

//void HandControl_Form::connectionStatusChanged()
//{
//    //    if (module->server->isConnected()) {
//    //        ui->connectionStatus->setText("Connected.");
//    //    } else {
//    //        ui->connectionStatus->setText("Not connected.");
//    //    }
//}

////void HandControl_Form::dataChanged(RobotModule *m)
////{

////    int forwardSpeed = m->getDataValue("forwardSpeed").toInt();
////    int angularSpeed = m->getDataValue("angularSpeed").toInt();
////    int speedUpDown = m->getDataValue("speedUpDown").toInt();

////    ui->sliderFw->setValue(forwardSpeed);
////    ui->sliderLR->setValue(angularSpeed);
////    ui->sliderUD->setValue(speedUpDown);
////}

//void HandControl_Form::on_forwardButton_clicked()
//{
//    fwd =  maxForwardSpeed;
//}

//void HandControl_Form::on_leftButton_clicked()
//{
//    ang = -maxAngularSpeed;
//}

//void HandControl_Form::on_backwardButton_clicked()
//{
//    fwd =  -maxForwardSpeed;
//}

//void HandControl_Form::on_rightButton_clicked()
//{
//    ang = maxAngularSpeed;
//}

//void HandControl_Form::on_upButton_clicked()
//{
//    upDownSpeed = upDownSpeed - stepsize;
//    if(upDownSpeed < 0.0){
//        upDownSpeed = 0.0;
//    }
//    dep = upDownSpeed;
//}

//void HandControl_Form::on_downButton_clicked()
//{
//    upDownSpeed = upDownSpeed + stepsize;
//    dep = upDownSpeed;
//}

////void HandControl_Form::resetForSpeeds(){
////    module->addData("speed forward", 0);
////    fwd = 0;
////}

////void HandControl_Form::resetAngSpeeds(){
////    module->addData("speed angular", 0);
////    ang = 0;
////}

//void HandControl_Form::on_resetDepthButton_clicked()
//{
//    upDownSpeed = 0.0;
//    module->addData("speed up down", upDownSpeed);
//    dep = upDownSpeed;
//}

//void HandControl_Form::setSpeeds(){
//    updateControls(fwd, ang, dep);
//    fwd = 0;
//    ang = 0;
//}
