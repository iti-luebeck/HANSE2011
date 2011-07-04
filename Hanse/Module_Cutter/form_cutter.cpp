#include "form_cutter.h"
#include "ui_form_cutter.h"

FormCutter::FormCutter(Module_Cutter* module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_Cutter)
{
    ui->setupUi(this);
    this->module = module;

    ui->i2cAddress->setText(module->getSettingsValue("i2cAddress").toString());
    ui->timeout->setText(module->getSettingsValue("timeout").toString());

}

FormCutter::~FormCutter()
{
    delete ui;
}

void FormCutter::changeEvent(QEvent *e)
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

void FormCutter::on_save_clicked()
{
    module->setSettingsValue("i2cAddress", ui->i2cAddress->text());
    module->setSettingsValue("timeout",ui->timeout->text());
}
