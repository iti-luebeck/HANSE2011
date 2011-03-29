#include "tcl_form.h"
#include "ui_tcl_form.h"

#include <qwt-qt4/qwt_legend.h>
#include <Framework/pidwidget.h>

TCL_Form::TCL_Form(Module_ThrusterControlLoop *module, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TCL_Form)
{
    ui->setupUi(this);
    this->module = module;

    ui->kpEdit->setText(module->getSettingsValue("Kp").toString());
    ui->tiEdit->setText(module->getSettingsValue("Ti").toString());
    ui->tdEdit->setText(module->getSettingsValue("Td").toString());
    ui->neutralEdit->setText(module->getSettingsValue("neutralSpeed").toString());
    ui->minEdit->setText(module->getSettingsValue("minSpeed").toString());
    ui->maxEdit->setText(module->getSettingsValue("maxSpeed").toString());
    ui->forceUnpauseError->setText(module->getSettingsValue("forceUnpauseError").toString());

    ui->horizSpM_exp->setChecked( module->getSettingsValue("horizSpM_exp").toBool() );

    connect(module, SIGNAL(dataChanged(RobotModule*)), this, SLOT(dataChanged(RobotModule*)));

    QLayout *l = new QBoxLayout(QBoxLayout::LeftToRight, ui->frame);
    PIDWidget *pid = new PIDWidget(module->pidController, this);
    l->addWidget(pid);

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
    module->setSettingsValue("Kp", ui->kpEdit->text().toFloat());
    module->setSettingsValue("Ti", ui->tiEdit->text().toFloat());
    module->setSettingsValue("Td", ui->tdEdit->text().toFloat());
    module->setSettingsValue("neutralSpeed", ui->neutralEdit->text().toFloat());
    module->setSettingsValue("minSpeed", ui->minEdit->text().toFloat());
    module->setSettingsValue("maxSpeed", ui->maxEdit->text().toFloat());
    module->setSettingsValue("forceUnpauseError", ui->forceUnpauseError->text());

    module->setSettingsValue("horizSpM_exp", ui->horizSpM_exp->isChecked() );
    module->setSettingsValue("ignoreHealth", ui->ignoreHealth->isChecked() );

    module->updateConstantsFromInitNow();
}

void TCL_Form::dataChanged(RobotModule *mod)
{
    if (!ui->updatePlot->isChecked()) {
        return;
    }
}
