#include "imu_form.h"
#include "ui_imu_form.h"
#include "module_imu.h"

IMU_Form::IMU_Form(Module_IMU *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IMU_Form)
{
    ui->setupUi(this);
    this->module = module;
    ui->frequency->setText(module->getSettingsValue("frequency").toString());
    ui->ssLine->setValue(module->getSettingsValue("ssLine").toInt());
    ui->spiSpeed->setValue(module->getSettingsValue("spiSpeed").toInt());
    ui->biasComp->setChecked(module->getSettingsValue("biasComp").toBool());
    ui->originAllign->setChecked(module->getSettingsValue("originAllign").toBool());
    ui->smplTimeBase->setValue(module->getSettingsValue("smplTimeBase").toInt());
    ui->smplTimeMult->setValue(module->getSettingsValue("smplTimeMult").toInt());
    ui->filterTaps->setValue(module->getSettingsValue("filterTaps").toInt());
    ui->gyroSens->setCurrentIndex(ui->gyroSens->findText(module->getSettingsValue("gyroSens").toString()));

    qRegisterMetaType<QVariant>("QVariant");
    QObject::connect(this,SIGNAL(doPrecisionCalib()),module,SLOT(doPrecisionCalib()));
    QObject::connect(this,SIGNAL(doNullCalib()),module,SLOT(doNullCalib()));
    QObject::connect(this,SIGNAL(reset()),module,SLOT(reset()));
    QObject::connect(this,SIGNAL(newDataValue(QString,QVariant)),module,SLOT(addData(QString,QVariant)));
    QObject::connect(this,SIGNAL(newSettingsValue(QString,QVariant)),module,SLOT(setSettingsValue(QString,QVariant)));
    // big hack: wait until data is read.
    timer.singleShot(1000,this, SLOT(updateBiasFields()));
}

IMU_Form::~IMU_Form()
{
    delete ui;
}

void IMU_Form::changeEvent(QEvent *e)
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

void IMU_Form::on_save_clicked()
{
    emit newSettingsValue("frequency",ui->frequency->text().toInt());
    emit newSettingsValue("ssLine",ui->ssLine->text().toInt());
    emit newSettingsValue("spiSpeed",ui->spiSpeed->value());
    emit newSettingsValue("biasComp", ui->biasComp->isChecked());
    emit newSettingsValue("originAllign", ui->originAllign->isChecked());
    emit newSettingsValue("smplTimeBase", ui->smplTimeBase->text());
    emit newSettingsValue("smplTimeMult", ui->smplTimeMult->text());
    emit newSettingsValue("filterTaps", ui->filterTaps->text());
    emit newSettingsValue("gyroSens", ui->gyroSens->currentText());
//    module->setSettingsValue("frequency",ui->frequency->text().toInt());
//    module->setSettingsValue("ssLine",ui->ssLine->text().toInt());
//    module->setSettingsValue("spiSpeed",ui->spiSpeed->value());
//    module->setSettingsValue("biasComp", ui->biasComp->isChecked());
//    module->setSettingsValue("originAllign", ui->originAllign->isChecked());
//    module->setSettingsValue("smplTimeBase", ui->smplTimeBase->text());
//    module->setSettingsValue("smplTimeMult", ui->smplTimeMult->text());
//    module->setSettingsValue("filterTaps", ui->filterTaps->text());
//    module->setSettingsValue("gyroSens", ui->gyroSens->currentText());
//    module->reset();
    emit reset();
}

void IMU_Form::on_calibNull_clicked()
{
//    module->doNullCalib();
    emit doNullCalib();
    timer.singleShot(1000,this, SLOT(updateBiasFields()));
}

void IMU_Form::on_calibPrecise_clicked()
{
    updateBiasFields();
//    module->doPrecisionCalib();
    emit doPrecisionCalib();
    timer.singleShot(30000,this, SLOT(updateBiasFields()));
}

void IMU_Form::updateBiasFields()
{
    ui->gyroXbias->setText(module->getDataValue("biasGyroX").toString());
    ui->gyroYbias->setText(module->getDataValue("biasGyroY").toString());
    ui->gyroZbias->setText(module->getDataValue("biasGyroZ").toString());
    ui->accelXbias->setText(module->getDataValue("biasAccelX").toString());
    ui->accelYbias->setText(module->getDataValue("biasAccelY").toString());
    ui->accelZbias->setText(module->getDataValue("biasAccelZ").toString());
}
