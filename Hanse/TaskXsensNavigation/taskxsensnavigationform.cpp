#include "taskxsensnavigationform.h"
#include "ui_taskxsensnavigationform.h"

TaskXsensNavigationForm::TaskXsensNavigationForm(TaskXsensNavigation *txn, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskXsensNavigationForm)
{
    ui->setupUi(this);
    this->taskxsensnavigation = txn;

    connect(taskxsensnavigation,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

    // Show settings from taskxsensfollowing
    this->ui->forwardInput->setText(this->taskxsensnavigation->getSettingsValue("forwardSpeed").toString());
    this->ui->angularInput->setText(this->taskxsensnavigation->getSettingsValue("angularSpeed").toString());

    this->ui->startInput->setText(this->taskxsensnavigation->getSettingsValue("startNavigation").toString());
    this->ui->startToleranceInput->setText(this->taskxsensnavigation->getSettingsValue("startTolerance").toString());

    this->ui->targetInput->setText(this->taskxsensnavigation->getSettingsValue("targetNavigation").toString());
    this->ui->targetToleranceInput->setText(this->taskxsensnavigation->getSettingsValue("targetTolerance").toString());
    this->ui->enableTimerBox->setChecked(this->taskxsensnavigation->getSettingsValue("timerActivated").toBool());
    this->ui->enableLoopBox->setChecked(this->taskxsensnavigation->getSettingsValue("loopActivated").toBool());

    this->ui->taskStopInput->setText(this->taskxsensnavigation->getSettingsValue("taskStopTime").toString());
    this->ui->signalInput->setText(this->taskxsensnavigation->getSettingsValue("signalTimer").toString());
}

TaskXsensNavigationForm::~TaskXsensNavigationForm()
{
    delete ui;
}

void TaskXsensNavigationForm::changeEvent(QEvent *e)
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

void TaskXsensNavigationForm::on_applyButton_clicked(){

    this->taskxsensnavigation->setSettingsValue("forwardSpeed" ,this->ui->forwardInput->text());
    this->taskxsensnavigation->setSettingsValue("angularSpeed" ,this->ui->angularInput->text());

    this->taskxsensnavigation->setSettingsValue("startNavigation", this->ui->targetInput->text());
    this->taskxsensnavigation->setSettingsValue("startTolerance", this->ui->startToleranceInput->text());

    this->taskxsensnavigation->setSettingsValue("targetNavigation", this->ui->startInput->text());
    this->taskxsensnavigation->setSettingsValue("targetTolerance", this->ui->targetToleranceInput->text());

    this->taskxsensnavigation->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
    this->taskxsensnavigation->setSettingsValue("loopActivated", this->ui->enableLoopBox->isChecked());

    this->taskxsensnavigation->setSettingsValue("taskStopTime", this->ui->taskStopInput->text());
    this->taskxsensnavigation->setSettingsValue("signalTimer", this->ui->signalInput->text());
}


