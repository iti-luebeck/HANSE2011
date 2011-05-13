#include "xsens_form.h"
#include "ui_xsens_form.h"
#include <Module_XsensMTi/module_xsensmti.h>

Xsens_Form::Xsens_Form(Module_XsensMTi *mti, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Xsens_Form)
{
    ui->setupUi(this);
    this->mti = mti;

    ui->updateRateEdit->setText(mti->getSettingsValue("updaterate").toString());
    ui->portEdit->setText(mti->getSettingsValue("port").toString());
    ui->baudRateEdit->setText(mti->getSettingsValue("baudrate").toString());
    ui->checkBox->setChecked(mti->getSettingsValue("upsidedown").toBool());
}

Xsens_Form::~Xsens_Form()
{
    delete ui;
}

void Xsens_Form::on_pushButton_clicked()
{
    int updateRate = ui->updateRateEdit->text().toInt();
    QString portName = ui->portEdit->text();
    int baudRate = ui->baudRateEdit->text().toInt();
    bool upsidedown = ui->checkBox->isChecked();

    mti->setSettingsValue("updaterate", updateRate);
    mti->setSettingsValue("port", portName);
    mti->setSettingsValue("baudrate", baudRate);
    mti->setSettingsValue("upsidedown", upsidedown);
    QTimer::singleShot(0, mti, SLOT(reset()));
}
