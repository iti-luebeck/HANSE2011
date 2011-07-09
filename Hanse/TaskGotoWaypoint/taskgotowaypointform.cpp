#include "taskgotowaypointform.h"
#include "ui_taskgotowaypointform.h"

TaskGotoWaypointForm::TaskGotoWaypointForm(TaskGotoWaypoint *tgw, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskGotoWaypointForm)
{
    ui->setupUi(this);
    this->taskgotowaypoint = tgw;

    connect(taskgotowaypoint,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

    // Task settings
    this->ui->enableTimerBox->setChecked(this->taskgotowaypoint->getSettingsValue("timerActivated").toBool());
    this->ui->taskStopInput->setText(this->taskgotowaypoint->getSettingsValue("taskStopTime").toString());

    // Waypoint settings
    this->ui->point1->setText(taskgotowaypoint->getSettingsValue("point1").toString());
    this->ui->use1->setChecked(taskgotowaypoint->getSettingsValue("use1").toBool());
    this->ui->point2->setText(taskgotowaypoint->getSettingsValue("point2").toString());
    this->ui->use2->setChecked(taskgotowaypoint->getSettingsValue("use2").toBool());
    this->ui->point3->setText(taskgotowaypoint->getSettingsValue("point3").toString());
    this->ui->use3->setChecked(taskgotowaypoint->getSettingsValue("use3").toBool());
    this->ui->point4->setText(taskgotowaypoint->getSettingsValue("point4").toString());
    this->ui->use4->setChecked(taskgotowaypoint->getSettingsValue("use4").toBool());
    this->ui->point5->setText(taskgotowaypoint->getSettingsValue("point5").toString());
    this->ui->use5->setChecked(taskgotowaypoint->getSettingsValue("use5").toBool());
    this->ui->point6->setText(taskgotowaypoint->getSettingsValue("point6").toString());
    this->ui->use6->setChecked(taskgotowaypoint->getSettingsValue("use6").toBool());

}

TaskGotoWaypointForm::~TaskGotoWaypointForm()
{
    delete ui;
}

void TaskGotoWaypointForm::changeEvent(QEvent *e)
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

void TaskGotoWaypointForm::on_applyButton_clicked(){
    // Task Settings
    this->taskgotowaypoint->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
    this->taskgotowaypoint->setSettingsValue("taskStopTime", this->ui->taskStopInput->text());

    // Waypoint settings
    this->taskgotowaypoint->setSettingsValue("point1", ui->point1->text());
    this->taskgotowaypoint->setSettingsValue("use1", ui->use1->isChecked());
    this->taskgotowaypoint->setSettingsValue("point2", ui->point2->text());
    this->taskgotowaypoint->setSettingsValue("use2", ui->use2->isChecked());
    this->taskgotowaypoint->setSettingsValue("point3", ui->point3->text());
    this->taskgotowaypoint->setSettingsValue("use3", ui->use3->isChecked());
    this->taskgotowaypoint->setSettingsValue("point4", ui->point4->text());
    this->taskgotowaypoint->setSettingsValue("use4", ui->use4->isChecked());
    this->taskgotowaypoint->setSettingsValue("point5", ui->point5->text());
    this->taskgotowaypoint->setSettingsValue("use5", ui->use5->isChecked());
    this->taskgotowaypoint->setSettingsValue("point6", ui->point6->text());
    this->taskgotowaypoint->setSettingsValue("use6", ui->use6->isChecked());

}

