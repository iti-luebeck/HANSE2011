#include "form.h"
#include "ui_form.h"

Form::Form(Module_UID* module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->uidId->setText(module->getSettings().value("uidId", DEFAULT_UID_ID).toString());
}

Form::~Form()
{
    delete ui;
}

void Form::changeEvent(QEvent *e)
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

void Form::on_save_clicked()
{
    module->getSettings().setValue("uidId", ui->uidId->text());
}
