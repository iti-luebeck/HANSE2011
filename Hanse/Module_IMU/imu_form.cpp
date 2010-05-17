#include "imu_form.h"
#include "ui_imu_form.h"
#include "module_imu.h"

IMU_Form::IMU_Form(Module_IMU *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IMU_Form)
{
    ui->setupUi(this);
    this->module = module;
    ui->frequency->setText(module->getSettings().value("frequency").toString());
    ui->ssLine->setValue(module->getSettings().value("ssLine").toInt());
    ui->spiSpeed->setValue(module->getSettings().value("spiSpeed").toInt());
    ui->biasComp->setChecked(module->getSettings().value("biasComp").toBool());
    ui->originAllign->setChecked(module->getSettings().value("originAllign").toBool());
    ui->smplTimeBase->setValue(module->getSettings().value("smplTimeBase").toInt());
    ui->smplTimeMult->setValue(module->getSettings().value("smplTimeMult").toInt());
    ui->filterTaps->setValue(module->getSettings().value("filterTaps").toInt());
    ui->gyroSens->setCurrentIndex(ui->gyroSens->findText(module->getSettings().value("gyroSens").toString()));

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
    module->getSettings().setValue("frequency",ui->frequency->text().toInt());
    module->getSettings().setValue("ssLine",ui->ssLine->text().toInt());
    module->getSettings().setValue("spiSpeed",ui->spiSpeed->value());
    module->getSettings().setValue("biasComp", ui->biasComp->isChecked());
    module->getSettings().setValue("originAllign", ui->originAllign->isChecked());
    module->getSettings().setValue("smplTimeBase", ui->smplTimeBase->text());
    module->getSettings().setValue("smplTimeMult", ui->smplTimeMult->text());
    module->getSettings().setValue("filterTaps", ui->filterTaps->text());
    module->getSettings().setValue("gyroSens", ui->gyroSens->currentText());
    module->reset();
}

void IMU_Form::on_calibNull_clicked()
{
    module->doNullCalib();
    timer.singleShot(1000,this, SLOT(updateBiasFields()));
}

void IMU_Form::on_calibPrecise_clicked()
{
    updateBiasFields();
    module->doPrecisionCalib();
    timer.singleShot(30000,this, SLOT(updateBiasFields()));
}

void IMU_Form::updateBiasFields()
{
    ui->gyroXbias->setText(module->getData().value("biasGyroX").toString());
    ui->gyroYbias->setText(module->getData().value("biasGyroY").toString());
    ui->gyroZbias->setText(module->getData().value("biasGyroZ").toString());
    ui->accelXbias->setText(module->getData().value("biasAccelX").toString());
    ui->accelYbias->setText(module->getData().value("biasAccelY").toString());
    ui->accelZbias->setText(module->getData().value("biasAccelZ").toString());
}
