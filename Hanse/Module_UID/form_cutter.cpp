#include "form_cutter.h"
#include "ui_form_cutter.h"

FormCutter::FormCutter(Module_Cutter* module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_Cutter)
{
    ui->setupUi(this);
    this->module = module;

    ui->uidId->setText(module->getSettingsValue("uidId").toString());
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
    module->setSettingsValue("uidId", ui->uidId->text());
    module->setSettingsValue("timeout", ui->timeout->text());
}

void FormCutter::on_scan_clicked()
{
    QVector<unsigned char> slaves = module->I2C_Scan();
    ui->slaves->clear();
    foreach (unsigned int slave, slaves) {
        ui->slaves->addItem("0x"+QString::number(slave,16));
    }
}
