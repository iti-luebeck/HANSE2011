#include "imu_form.h"
#include "ui_imu_form.h"
#include "module_imu.h"

IMU_Form::IMU_Form(Module_IMU *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::IMU_Form)
{
    ui->setupUi(this);
    this->module = module;
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
    module->reset();
}

void IMU_Form::on_calibNull_clicked()
{

}
