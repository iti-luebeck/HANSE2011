#include "taskwallnavigationform.h"
#include "ui_taskwallnavigationform.h"

TaskWallNavigationForm::TaskWallNavigationForm(TaskWallNavigation *twn, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskWallNavigationForm)
{
    ui->setupUi(this);
    this->taskwallnavigation = twn;

    connect(taskwallnavigation,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

    // Show settings from taskwallfollowing
    this->ui->forwardInput->setText(this->taskwallnavigation->getSettingsValue("forwardSpeed").toString());
    this->ui->angularInput->setText(this->taskwallnavigation->getSettingsValue("angularSpeed").toString());
    this->ui->desiredDistanceInput->setText(this->taskwallnavigation->getSettingsValue("desiredDistance").toString());
    this->ui->corridorInput->setText(this->taskwallnavigation->getSettingsValue("corridorWidth").toString());
    this->ui->durationInput->setText(this->taskwallnavigation->getSettingsValue("taskDuration").toString());


    this->ui->startInput->setText(this->taskwallnavigation->getSettingsValue("startNavigation").toString());
    this->ui->startToleranceInput->setText(this->taskwallnavigation->getSettingsValue("startTolerance").toString());

    this->ui->targetInput->setText(this->taskwallnavigation->getSettingsValue("targetNavigation").toString());
    this->ui->targetToleranceInput->setText(this->taskwallnavigation->getSettingsValue("targetTolerance").toString());
    this->ui->enableTimerBox->setChecked(this->taskwallnavigation->getSettingsValue("timerActivated").toBool());
    this->ui->enableLoopBox->setChecked(this->taskwallnavigation->getSettingsValue("loopActivated").toBool());
}

TaskWallNavigationForm::~TaskWallNavigationForm()
{
    delete ui;
}

void TaskWallNavigationForm::changeEvent(QEvent *e)
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

void TaskWallNavigationForm::on_applyButton_clicked(){
    // Update taskwallfollowing settings
    this->taskwallnavigation->setSettingsValue("forwardSpeed" ,this->ui->forwardInput->text());
    this->taskwallnavigation->setSettingsValue("angularSpeed" ,this->ui->angularInput->text());
    this->taskwallnavigation->setSettingsValue("desiredDistance" ,this->ui->desiredDistanceInput->text());
    this->taskwallnavigation->setSettingsValue("corridorWidth" ,this->ui->corridorInput->text());
    this->taskwallnavigation->setSettingsValue("taskDuration", this->ui->durationInput->text());

    this->taskwallnavigation->setSettingsValue("startNavigation", this->ui->targetInput->text());
    this->taskwallnavigation->setSettingsValue("startTolerance", this->ui->startToleranceInput->text());

    this->taskwallnavigation->setSettingsValue("targetNavigation", this->ui->startInput->text());
    this->taskwallnavigation->setSettingsValue("targetTolerance", this->ui->targetToleranceInput->text());

    this->taskwallnavigation->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
    this->taskwallnavigation->setSettingsValue("loopActivated", this->ui->enableLoopBox->isChecked());

    qDebug("Timer?");
    qDebug()<<this->ui->enableTimerBox->isChecked();

    qDebug("Loop?");
    qDebug()<<this->ui->enableLoopBox->isChecked();
}


