#include "form_uid.h"
#include "ui_form_uid.h"

FormUID::FormUID(Module_UID* module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_UID)
{
    ui->setupUi(this);
    this->module = module;

    ui->uidId->setText(module->getSettings().value("uidId").toString());
}

FormUID::~FormUID()
{
    delete ui;
}

void FormUID::changeEvent(QEvent *e)
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

void FormUID::on_save_clicked()
{
    module->getSettings().setValue("uidId", ui->uidId->text());
}

void FormUID::on_scan_clicked()
{
    QVector<unsigned char> slaves = module->I2C_Scan();
    ui->slaves->clear();
    foreach (unsigned int slave, slaves) {
        ui->slaves->addItem("0x"+QString::number(slave,16));
    }
}
