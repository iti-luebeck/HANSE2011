#include "tcl_form.h"
#include "ui_tcl_form.h"

TCL_Form::TCL_Form(Module_ThrusterControlLoop *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TCL_Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->p_up->setText(module->getSettings().value(      "p_up").toString());
    ui->p_down->setText(module->getSettings().value(    "p_down").toString());
    ui->maxSpU->setText(module->getSettings().value(    "maxSpU").toString());
    ui->maxSpD->setText(module->getSettings().value(    "maxSpD").toString());
    ui->neutrSpD->setText(module->getSettings().value(  "neutrSpD").toString());
    ui->maxDepthError->setText(module->getSettings().value(  "maxDepthError").toString());

    ui->horizSpM_exp->setChecked( module->getSettings().value("horizSpM_exp").toBool() );

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
    module->getSettings().setValue("p_up",      ui->p_up->text().toFloat());
    module->getSettings().setValue("p_down",    ui->p_down->text().toFloat());
    module->getSettings().setValue("maxSpU",    ui->maxSpU->text().toFloat());
    module->getSettings().setValue("maxSpD",    ui->maxSpD->text().toFloat());
    module->getSettings().setValue("neutrSpD",  ui->neutrSpD->text().toFloat());
    module->getSettings().setValue("maxDepthError",  ui->maxDepthError->text().toFloat());

    module->getSettings().setValue("horizSpM_exp", ui->horizSpM_exp->isChecked() );

    module->updateConstantsFromInitNow();
}
