#include "taskpipefollowingform.h"
#include "ui_taskpipefollowingform.h"

TaskPipeFollowingForm::TaskPipeFollowingForm(TaskPipeFollowing *tpf, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskPipeFollowingForm)
{
    ui->setupUi(this);
    this->taskpipefollowing = tpf;

    connect(taskpipefollowing,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

    // Pipefollowing settings
    this->ui->badFramesLineEdit->setText(taskpipefollowing->getSettingsValue("badFrames").toString());
    this->ui->camHeightLineEdit->setText(taskpipefollowing->getSettingsValue("camHeight").toString());
    this->ui->camWidthLineEdit->setText(taskpipefollowing->getSettingsValue("camWidth").toString());
    this->ui->deltaAnglePipeLineEdit->setText(taskpipefollowing->getSettingsValue("deltaAngle").toString());
    this->ui->deltaDistPipeLineEdit->setText(taskpipefollowing->getSettingsValue("deltaDist").toString());
    this->ui->grayRadioButton->setChecked(taskpipefollowing->getSettingsValue("convColor").toInt() == 4);
    this->ui->hRadioButton->setChecked(taskpipefollowing->getSettingsValue("convColor").toInt() == 1);
    this->ui->hsvRadioButton->setChecked(taskpipefollowing->getSettingsValue("convColor").toInt() == 0);
    this->ui->kpAngleLineEdit->setText(taskpipefollowing->getSettingsValue("kpAngle").toString());
    this->ui->kpDistLineEdit->setText(taskpipefollowing->getSettingsValue("kpDist").toString());
    this->ui->maxDistLineEdti->setText(taskpipefollowing->getSettingsValue("maxDistance").toString());
    this->ui->robCenterXLineEdit->setText(taskpipefollowing->getSettingsValue("robCenterX").toString());
    this->ui->robCenterYLineEdit->setText(taskpipefollowing->getSettingsValue("robCenterY").toString());
    this->ui->sRadioButton->setChecked(taskpipefollowing->getSettingsValue("convColor").toInt() == 2);
    this->ui->speedFwLineEdit->setText(taskpipefollowing->getSettingsValue("fwSpeed").toString());
    this->ui->thresholdLineEdit->setText(taskpipefollowing->getSettingsValue("threshold").toString());
    this->ui->timer_LineEdit->setText(taskpipefollowing->getSettingsValue("timer").toString());
    this->ui->vRadioButton->setChecked(taskpipefollowing->getSettingsValue("convColor").toInt() == 3);
    this->ui->checkBox->setChecked(taskpipefollowing->getSettingsValue("enableUIOutput").toBool());
    this->ui->frameColorCB->setChecked(taskpipefollowing->getSettingsValue("frameOutput").toBool());

    // Task settings
    this->ui->enableTimerBox->setChecked(this->taskpipefollowing->getSettingsValue("timerActivated").toBool());
    this->ui->enableLoopBox->setChecked(this->taskpipefollowing->getSettingsValue("loopActivated").toBool());
    this->ui->taskStopInput->setText(this->taskpipefollowing->getSettingsValue("taskStopTime").toString());
    this->ui->signalInput->setText(this->taskpipefollowing->getSettingsValue("signalTimer").toString());

    // Navigation settings
    this->ui->navPipeStartInput->setText(this->taskpipefollowing->getSettingsValue("taskStartPoint").toString());

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
    // Pipefollowing settings
    this->taskpipefollowing->setSettingsValue("threshold",ui->thresholdLineEdit->text().toInt());
    this->taskpipefollowing->setSettingsValue("timer",ui->timer_LineEdit->text().toInt());
    this->taskpipefollowing->setSettingsValue("deltaDist",ui->deltaDistPipeLineEdit->text().toFloat());
    this->taskpipefollowing->setSettingsValue("deltaAngle",ui->deltaAnglePipeLineEdit->text().toFloat());
    this->taskpipefollowing->setSettingsValue("kpDist",ui->kpDistLineEdit->text().toFloat());
    this->taskpipefollowing->setSettingsValue("kpAngle",ui->kpAngleLineEdit->text().toFloat());
    this->taskpipefollowing->setSettingsValue("robCenterX",ui->robCenterXLineEdit->text().toDouble());
    this->taskpipefollowing->setSettingsValue("robCenterY",ui->robCenterYLineEdit->text().toDouble());
    this->taskpipefollowing->setSettingsValue("maxDistance",ui->maxDistLineEdti->text().toFloat());
    this->taskpipefollowing->setSettingsValue("fwSpeed",ui->speedFwLineEdit->text().toFloat());
    this->taskpipefollowing->setSettingsValue("camHeight",ui->camHeightLineEdit->text().toInt());
    this->taskpipefollowing->setSettingsValue("camWidth",ui->camWidthLineEdit->text().toInt());
    this->taskpipefollowing->setSettingsValue("badFrames",ui->badFramesLineEdit->text().toInt());
    this->taskpipefollowing->setSettingsValue("frameOutput",ui->frameColorCB->isChecked());

    if(ui->hRadioButton->isChecked()){
        this->taskpipefollowing->setSettingsValue("convColor",1);
    } else if(ui->sRadioButton->isChecked()){
        this->taskpipefollowing->setSettingsValue("convColor",2);
    } else if(ui->vRadioButton->isChecked()){
        this->taskpipefollowing->setSettingsValue("convColor",3);
    } else if(ui->grayRadioButton->isChecked()){
        this->taskpipefollowing->setSettingsValue("convColor",4);
    } else if(ui->hsvRadioButton->isChecked()){
        this->taskpipefollowing->setSettingsValue("convColor",0);
    }

    if(ui->checkBox->isChecked()){
        //QTimer::singleShot(0,&updateUI,SLOT(start()));
        taskpipefollowing->setSettingsValue("enableUIOutput",true);
    } else if(!ui->checkBox->isChecked()){
        //QTimer::singleShot(0,&updateUI,SLOT(stop()));
        taskpipefollowing->setSettingsValue("enableUIOutput",false);
    }

    // Task Settings
    this->taskpipefollowing->setSettingsValue("timerActivated", this->ui->enableTimerBox->isChecked());
    this->taskpipefollowing->setSettingsValue("loopActivated", this->ui->enableLoopBox->isChecked());
    this->taskpipefollowing->setSettingsValue("taskStopTime", this->ui->taskStopInput->text());
    this->taskpipefollowing->setSettingsValue("signalTimer", this->ui->signalInput->text());

    // Navigation settings
    this->taskpipefollowing->setSettingsValue("taskStartPoint", this->ui->navPipeStartInput->text());
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

