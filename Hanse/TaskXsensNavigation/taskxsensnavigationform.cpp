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
    this->ui->startInput->setText(this->taskxsensnavigation->getSettingsValue("startNavigation").toString());
    this->ui->startToleranceInput->setText(this->taskxsensnavigation->getSettingsValue("startTolerance").toString());

    this->ui->bInput->setText(this->taskxsensnavigation->getSettingsValue("bNavigation").toString());
    this->ui->bToleranceInput->setText(this->taskxsensnavigation->getSettingsValue("bTolerance").toString());

    this->ui->targetInput->setText(this->taskxsensnavigation->getSettingsValue("targetNavigation").toString());
    this->ui->targetToleranceInput->setText(this->taskxsensnavigation->getSettingsValue("targetTolerance").toString());

    this->ui->enableTimerBox->setChecked(this->taskxsensnavigation->getSettingsValue("timerActivated").toBool());
    this->ui->enableLoopBox->setChecked(this->taskxsensnavigation->getSettingsValue("loopActivated").toBool());

    this->ui->taskStopInput->setText(this->taskxsensnavigation->getSettingsValue("taskStopTime").toString());
    this->ui->signalInput->setText(this->taskxsensnavigation->getSettingsValue("signalTimer").toString());

    // Xsensfollow Settings
    this->ui->ffSpeed->setText(this->taskxsensnavigation->getSettingsValue("ffSpeed").toString());
    this->ui->kp->setText(this->taskxsensnavigation->getSettingsValue("kp").toString());
    this->ui->delta->setText(this->taskxsensnavigation->getSettingsValue("delta").toString());
    this->ui->driveTime->setText(this->taskxsensnavigation->getSettingsValue("driveTime").toString());
    this->ui->timerInput->setText(this->taskxsensnavigation->getSettingsValue("timer").toString());
    this->ui->waitInput->setText(this->taskxsensnavigation->getSettingsValue("waitTime").toString());

    // Turn180 Settings
    this->ui->hysteresisEdit->setText(this->taskxsensnavigation->getSettingsValue("hysteresis").toString());
    this->ui->pEdit->setText(this->taskxsensnavigation->getSettingsValue("p").toString());
    this->ui->degreeInput->setText(this->taskxsensnavigation->getSettingsValue("degree").toString());

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
    this->taskxsensnavigation->setSettingsValue("startNavigation", this->ui->startInput->text());
    this->taskxsensnavigation->setSettingsValue("startTolerance", this->ui->startToleranceInput->text());

    this->taskxsensnavigation->setSettingsValue("bNavigation", this->ui->bInput->text());
    this->taskxsensnavigation->setSettingsValue("bTolerance", this->ui->bToleranceInput->text());

    this->taskxsensnavigation->setSettingsValue("targetNavigation", this->ui->targetInput->text());
    this->taskxsensnavigation->setSettingsValue("targetTolerance", this->ui->targetToleranceInput->text());

    this->taskxsensnavigation->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
    this->taskxsensnavigation->setSettingsValue("loopActivated", this->ui->enableLoopBox->isChecked());

    this->taskxsensnavigation->setSettingsValue("taskStopTime", this->ui->taskStopInput->text());
    this->taskxsensnavigation->setSettingsValue("signalTimer", this->ui->signalInput->text());

    // Xsensfollow Settings
    this->taskxsensnavigation->setSettingsValue("ffSpeed", this->ui->ffSpeed->text());
    this->taskxsensnavigation->setSettingsValue("kp", this->ui->kp->text());
    this->taskxsensnavigation->setSettingsValue("delta", this->ui->delta->text());
    this->taskxsensnavigation->setSettingsValue("driveTime", this->ui->driveTime->text());
    this->taskxsensnavigation->setSettingsValue("timer", this->ui->timerInput->text());
    this->taskxsensnavigation->setSettingsValue("waitTime", this->ui->waitInput->text());

    // Turn180 Settings
    this->taskxsensnavigation->setSettingsValue("hysteresis", this->ui->hysteresisEdit->text());
    this->taskxsensnavigation->setSettingsValue("p", this->ui->pEdit->text());
    this->taskxsensnavigation->setSettingsValue("degree", this->ui->degreeInput->text());
}


