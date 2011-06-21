#include "taskpipefollowingform.h"
#include "ui_taskpipefollowingform.h"

TaskPipeFollowingForm::TaskPipeFollowingForm(TaskPipeFollowing *tpf, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskPipeFollowingForm)
{
    ui->setupUi(this);
    this->taskpipefollowing = tpf;

    connect(taskpipefollowing,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

    // Task settings
    this->ui->enableTimerBox->setChecked(this->taskpipefollowing->getSettingsValue("timerActivated").toBool());
    this->ui->taskStopInput->setText(this->taskpipefollowing->getSettingsValue("taskStopTime").toString());

    // Navigation settings
    this->ui->navTaskStartInput->setText(this->taskpipefollowing->getSettingsValue("taskStartPoint").toString());
    this->ui->navPipeStartInput->setText(this->taskpipefollowing->getSettingsValue("pipeStartPoint").toString());
    this->ui->goal1point1->setText(this->taskpipefollowing->getSettingsValue("goal1point1").toString());
    this->ui->goal1point2->setText(this->taskpipefollowing->getSettingsValue("goal1point2").toString());
    this->ui->goal2point1->setText(this->taskpipefollowing->getSettingsValue("goal2point1").toString());
    this->ui->goal2point2->setText(this->taskpipefollowing->getSettingsValue("goal2point2").toString());
    this->ui->goal3point1->setText(this->taskpipefollowing->getSettingsValue("goal3point1").toString());
    this->ui->goal3point2->setText(this->taskpipefollowing->getSettingsValue("goal3point2").toString());
    this->ui->gate1point->setText(this->taskpipefollowing->getSettingsValue("gate1point").toString());
    this->ui->gate2point->setText(this->taskpipefollowing->getSettingsValue("gate2point").toString());

    // Turn180 settings
    this->ui->hysteresisEdit->setText(this->taskpipefollowing->getSettingsValue("hysteresis").toString());
    this->ui->pEdit->setText(this->taskpipefollowing->getSettingsValue("p").toString());
    this->ui->degreeInput->setText(this->taskpipefollowing->getSettingsValue("degree").toString());

}

TaskPipeFollowingForm::~TaskPipeFollowingForm()
{
    delete ui;
}

void TaskPipeFollowingForm::changeEvent(QEvent *e)
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

void TaskPipeFollowingForm::on_applyButton_clicked(){
    // Task Settings
    this->taskpipefollowing->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
    this->taskpipefollowing->setSettingsValue("taskStopTime", this->ui->taskStopInput->text());

    // Navigation settings
    this->taskpipefollowing->setSettingsValue("taskStartPoint", this->ui->navTaskStartInput->text());
    this->taskpipefollowing->setSettingsValue("pipeStartPoint", this->ui->navPipeStartInput->text());
    this->taskpipefollowing->setSettingsValue("goal1point1", this->ui->goal1point1->text());
    this->taskpipefollowing->setSettingsValue("goal1point2", this->ui->goal1point2->text());
    this->taskpipefollowing->setSettingsValue("goal2point1", this->ui->goal2point1->text());
    this->taskpipefollowing->setSettingsValue("goal2point2", this->ui->goal2point2->text());
    this->taskpipefollowing->setSettingsValue("goal3point1", this->ui->goal3point1->text());
    this->taskpipefollowing->setSettingsValue("goal3point2", this->ui->goal3point2->text());
    this->taskpipefollowing->setSettingsValue("gate1point", this->ui->gate1point->text());
    this->taskpipefollowing->setSettingsValue("gate2point", this->ui->gate2point->text());

    // Turn180 Settings
    this->taskpipefollowing->setSettingsValue("hysteresis", this->ui->hysteresisEdit->text());
    this->taskpipefollowing->setSettingsValue("p", this->ui->pEdit->text());
    this->taskpipefollowing->setSettingsValue("degree", this->ui->degreeInput->text());
}

