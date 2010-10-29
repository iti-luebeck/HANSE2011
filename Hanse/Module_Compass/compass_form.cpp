#include "compass_form.h"
#include "ui_compass_form.h"

Compass_Form::Compass_Form(Module_Compass *m, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Compass_Form)
{
    ui->setupUi(this);
    module = m;

    connect(m, SIGNAL(dataChanged(RobotModule*)), this, SLOT(dataChanged(RobotModule*)));

    ui->frequency->setText(m->getSettingsValue("frequency").toString());
    ui->i2cAddress->setText(m->getSettingsValue("i2cAddress").toString());
    ui->orientation->setCurrentIndex(ui->orientation->findText(m->getSettingsValue("orientation").toString()));
    ui->devAngle->setValue(m->getSettingsValue("devAngle").toInt());
    ui->varAngle->setValue(m->getSettingsValue("varAngle").toInt());
    ui->iirFilter->setValue(m->getSettingsValue("iirFilter").toInt());
    ui->sampleRate->setCurrentIndex(ui->sampleRate->findText(m->getSettingsValue("sampleRate").toString()));
    ui->debug->setChecked(m->getSettingsValue("debug").toBool());

}

Compass_Form::~Compass_Form()
{
    delete ui;
}

void Compass_Form::changeEvent(QEvent *e)
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

void Compass_Form::on_save_clicked()
{
    module->setSettingsValue("frequency", ui->frequency->text());
    module->setSettingsValue("i2cAddress", ui->i2cAddress->text());
    module->setSettingsValue("orientation", ui->orientation->currentText());
    module->setSettingsValue("devAngle", ui->devAngle->text());
    module->setSettingsValue("varAngle", ui->varAngle->text());
    module->setSettingsValue("iirFilter", ui->iirFilter->text());
    module->setSettingsValue("sampleRate", ui->sampleRate->currentText());
    module->setSettingsValue("debug", ui->debug->isChecked());
    module->reset();
}

void Compass_Form::on_calibStart_clicked()
{
    module->startCalibration();
}

void Compass_Form::on_calibStop_clicked()
{
    module->stopCalibration();
}

void Compass_Form::dataChanged(RobotModule *module)
{
    uint8_t byte = module->getDataValue("reg_om1").toInt();
    ui->om_1_0->setChecked(byte & 0x01);
    ui->om_1_1->setChecked(byte & 0x02);
    ui->om_1_2->setChecked(byte & 0x04);
    ui->om_1_3->setChecked(byte & 0x08);
    ui->om_1_4->setChecked(byte & 0x10);
    ui->om_1_5->setChecked(byte & 0x20);
    ui->om_1_6->setChecked(byte & 0x40);
    ui->om_1_7->setChecked(byte & 0x80);
}
