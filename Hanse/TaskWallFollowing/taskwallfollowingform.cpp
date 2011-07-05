#include "taskwallfollowingform.h"
#include "ui_taskwallfollowingform.h"

TaskWallFollowingForm::TaskWallFollowingForm(TaskWallFollowing *twf, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskWallFollowingForm)
{
    ui->setupUi(this);
    this->taskwallfollowing = twf;

    connect(taskwallfollowing,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

    // Task settings
    this->ui->enableTimerBox->setChecked(this->taskwallfollowing->getSettingsValue("timerActivated").toBool());
    this->ui->taskStopInput->setText(this->taskwallfollowing->getSettingsValue("taskStopTime").toString());

    // Navigation settings
    this->ui->navTaskStartInput->setText(this->taskwallfollowing->getSettingsValue("taskStartPoint").toString());
    this->ui->navWallAdjustPoint->setText(this->taskwallfollowing->getSettingsValue("wallAdjustPoint").toString());
    this->ui->useAdjustPoint->setChecked(this->taskwallfollowing->getSettingsValue("useAdjustPoint").toBool());
    this->ui->goal1point1->setText(this->taskwallfollowing->getSettingsValue("goal1point1").toString());
    this->ui->goal1point2->setText(this->taskwallfollowing->getSettingsValue("goal1point2").toString());
    this->ui->apDistInput->setText(this->taskwallfollowing->getSettingsValue("apDist").toString());

    // Turn180 settings
    this->ui->hysteresisEdit->setText(this->taskwallfollowing->getSettingsValue("hysteresis").toString());
    this->ui->pEdit->setText(this->taskwallfollowing->getSettingsValue("p").toString());
    this->ui->degreeInput->setText(this->taskwallfollowing->getSettingsValue("degree").toString());
    this->ui->distP1Input->setText(this->taskwallfollowing->getSettingsValue("distP1Input").toString());
}

TaskWallFollowingForm::~TaskWallFollowingForm()
{
    delete ui;
}

void TaskWallFollowingForm::changeEvent(QEvent *e)
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

void TaskWallFollowingForm::on_applyButton_clicked(){
    // Task Settings
    this->taskwallfollowing->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
    this->taskwallfollowing->setSettingsValue("taskStopTime", this->ui->taskStopInput->text());

    // Navigation settings
    this->taskwallfollowing->setSettingsValue("taskStartPoint", this->ui->navTaskStartInput->text());
    this->taskwallfollowing->setSettingsValue("wallAdjustPoint", this->ui->navWallAdjustPoint->text());
    this->taskwallfollowing->setSettingsValue("useAdjustPoint", this->ui->useAdjustPoint->isChecked());
    this->taskwallfollowing->setSettingsValue("goal1point1", this->ui->goal1point1->text());
    this->taskwallfollowing->setSettingsValue("goal1point2", this->ui->goal1point2->text());
    this->taskwallfollowing->setSettingsValue("apDist", this->ui->apDistInput->text());
    this->taskwallfollowing->setSettingsValue("distP1Input", this->ui->distP1Input->text());

    // Turn180 Settings
    this->taskwallfollowing->setSettingsValue("hysteresis", this->ui->hysteresisEdit->text());
    this->taskwallfollowing->setSettingsValue("p", this->ui->pEdit->text());
    this->taskwallfollowing->setSettingsValue("degree", this->ui->degreeInput->text());
}

