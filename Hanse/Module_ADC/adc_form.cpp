#include "adc_form.h"
#include "ui_adc_form.h"

ADC_Form::ADC_Form(Module_ADC *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ADC_Form)
{
    ui->setupUi(this);
    this->module = module;
    ui->i2caddress->setText(module->getSettingsValue("i2cAddress").toString());
    ui->freq->setText(module->getSettingsValue("frequency").toString());
    ui->vagnd->setText(module->getSettingsValue("Vagnd").toString());
    ui->vref->setText(module->getSettingsValue("Vref").toString());
    ui->filtersize->setText(module->getSettingsValue("filtersize").toString());
}

ADC_Form::~ADC_Form()
{
    delete ui;
}

void ADC_Form::on_save_clicked()
{
    module->setSettingsValue("i2cAddress", ui->i2caddress->text().toInt());
    module->setSettingsValue("Vagnd", ui->vagnd->text().toFloat());
    module->setSettingsValue("Vref", ui->vref->text().toFloat());
    module->setSettingsValue("frequency", ui->freq->text().toInt());
    module->setSettingsValue("filtersize",ui->filtersize->text().toInt());
}
