#include "taskmidwatertargetform.h"
#include "ui_taskmidwatertargetform.h"

TaskMidwaterTargetForm::TaskMidwaterTargetForm(TaskMidwaterTarget *tmt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskMidwaterTargetForm)
{
    ui->setupUi(this);
    this->taskmidwatertarget = tmt;

    connect(taskmidwatertarget,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

    this->ui->taskStopInput->setText(this->taskmidwatertarget->getSettingsValue("taskStopTime").toString());
    this->ui->enableTimerBox->setChecked(this->taskmidwatertarget->getSettingsValue("timerActivated").toBool());
    this->ui->navTaskStartInput->setText(this->taskmidwatertarget->getSettingsValue("taskStartPoint").toString());
}

TaskMidwaterTargetForm::~TaskMidwaterTargetForm()
{
    delete ui;
}

void TaskMidwaterTargetForm::changeEvent(QEvent *e)
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

void TaskMidwaterTargetForm::on_applyButton_clicked(){
    this->taskmidwatertarget->setSettingsValue("taskStartPoint", this->ui->navTaskStartInput->text());
    this->taskmidwatertarget->setSettingsValue("taskStopTime", this->ui->taskStopInput->text());
    this->taskmidwatertarget->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
}

