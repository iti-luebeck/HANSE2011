#include "thruster_form.h"
#include "ui_thruster_form.h"

Thruster_Form::Thruster_Form(Module_Thruster *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Thruster_Form)
{
    thruster = module;
    ui->setupUi(this);

    ui->i2cAddress->setText(thruster->getSettingsValue("i2cAddress").toString());
    ui->channel->setText(thruster->getSettingsValue("channel").toString());
    ui->multi->setText(thruster->getSettingsValue("multiplicator").toString());
    ui->frequency->setText(thruster->getSettingsValue("frequency").toString());
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
    thruster->setSettingsValue("i2cAddress", ui->i2cAddress->text().toInt(0,0));
    thruster->setSettingsValue("channel", ui->channel->text().toInt(0,0));
    thruster->setSettingsValue("multiplicator", ui->multi->text().toInt(0,0));
    thruster->setSettingsValue("frequency", ui->frequency->text().toInt());
    QTimer::singleShot(0, thruster, SLOT(reset()));
}
