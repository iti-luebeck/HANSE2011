#include "thruster_form.h"
#include "ui_thruster_form.h"

Thruster_Form::Thruster_Form(Module_Thruster *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Thruster_Form)
{
    thruster = module;
    ui->setupUi(this);

    ui->i2cAddress->setText(thruster->getSettings().value("i2cAddress").toString());
    ui->channel->setText(thruster->getSettings().value("channel").toString());
    ui->multi->setText(thruster->getSettings().value("multiplicator").toString());
}

Thruster_Form::~Thruster_Form()
{
    delete ui;
}

void Thruster_Form::changeEvent(QEvent *e)
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

void Thruster_Form::on_save_clicked()
{
    thruster->getSettings().setValue("i2cAddress", ui->i2cAddress->text().toInt(0,0));
    thruster->getSettings().setValue("channel", ui->channel->text().toInt(0,0));
    thruster->getSettings().setValue("multiplicator", ui->multi->text().toInt(0,0));
}
