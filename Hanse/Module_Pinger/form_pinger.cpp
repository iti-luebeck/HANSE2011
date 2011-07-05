#include "form_pinger.h"
#include "ui_form_pinger.h"

FormPinger::FormPinger(Module_Pinger* module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_Pinger)
{
    ui->setupUi(this);
    this->module = module;

}

FormPinger::~FormPinger()
{
    delete ui;
}

void FormPinger::changeEvent(QEvent *e)
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

void FormPinger::on_save_clicked()
{


}
