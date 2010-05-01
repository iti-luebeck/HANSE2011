#include "tcl_form.h"
#include "ui_tcl_form.h"

TCL_Form::TCL_Form(Module_ThrusterControlLoop *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TCL_Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->p->setText(module->getSettings().value("p").toString());
    ui->i->setText(module->getSettings().value("i").toString());
    ui->d->setText(module->getSettings().value("d").toString());
    ui->gap->setText(module->getSettings().value("gap").toString());
}

TCL_Form::~TCL_Form()
{
    delete ui;
}

void TCL_Form::changeEvent(QEvent *e)
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

void TCL_Form::on_save_clicked()
{
    module->getSettings().setValue("p", ui->p->text().toFloat());
    module->getSettings().setValue("i", ui->i->text().toFloat());
    module->getSettings().setValue("d", ui->d->text().toFloat());
    module->getSettings().setValue("gap", ui->gap->text().toFloat());
}
