#include "taskpipefollowingform.h"
#include "ui_taskpipefollowingform.h"

TaskPipeFollowingForm::TaskPipeFollowingForm(TaskPipeFollowing *tpf, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskPipeFollowingForm)
{
    ui->setupUi(this);
    this->taskpipefollowing = tpf;

    connect(taskpipefollowing,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    connect(taskpipefollowing,SIGNAL(setDescriptionSignal()),this,SLOT(on_applyButton_clicked()));
    connect(this,SIGNAL(newSchDesSignal(QString, QString)),taskpipefollowing,SLOT(newSchDesSlot(QString, QString)));

    // Show settings from taskpipefollowing
    this->ui->badFramesLineEdit1->setText(taskpipefollowing->getSettingsValue("badFrames1").toString());
    this->ui->camHeightLineEdit1->setText(taskpipefollowing->getSettingsValue("camHeight1").toString());
    this->ui->camWidthLineEdit1->setText(taskpipefollowing->getSettingsValue("camWidth1").toString());
    this->ui->deltaAnglePipeLineEdit1->setText(taskpipefollowing->getSettingsValue("deltaAngle1").toString());
    this->ui->deltaDistPipeLineEdit1->setText(taskpipefollowing->getSettingsValue("deltaDist1").toString());
    this->ui->grayRadioButton1->setChecked(taskpipefollowing->getSettingsValue("convColor1").toInt() == 4);
    this->ui->hRadioButton1->setChecked(taskpipefollowing->getSettingsValue("convColor1").toInt() == 1);
    this->ui->hsvRadioButton1->setChecked(taskpipefollowing->getSettingsValue("convColor1").toInt() == 0);
    this->ui->kpAngleLineEdit1->setText(taskpipefollowing->getSettingsValue("kpAngle1").toString());
    this->ui->kpDistLineEdit1->setText(taskpipefollowing->getSettingsValue("kpDist1").toString());
    this->ui->maxDistLineEdti1->setText(taskpipefollowing->getSettingsValue("maxDistance1").toString());
    this->ui->robCenterXLineEdit1->setText(taskpipefollowing->getSettingsValue("robCenterX1").toString());
    this->ui->robCenterYLineEdit1->setText(taskpipefollowing->getSettingsValue("robCenterY1").toString());
    this->ui->sRadioButton1->setChecked(taskpipefollowing->getSettingsValue("convColor1").toInt() == 2);
    this->ui->speedFwLineEdit1->setText(taskpipefollowing->getSettingsValue("fwSpeed1").toString());
    this->ui->thresholdLineEdit1->setText(taskpipefollowing->getSettingsValue("threshold1").toString());
    this->ui->timer_LineEdit1->setText(taskpipefollowing->getSettingsValue("timer1").toString());
    this->ui->vRadioButton1->setChecked(taskpipefollowing->getSettingsValue("convColor1").toInt() == 3);

    this->ui->checkBox1->setChecked(taskpipefollowing->getSettingsValue("enableUIOutput1").toBool());
    this->ui->frameColorCB1->setChecked(taskpipefollowing->getSettingsValue("frameOutput1").toBool());

    this->ui->durationInput1->setText(this->taskpipefollowing->getSettingsValue("taskDuration1").toString());
    this->ui->descriptionInput1->setText(this->taskpipefollowing->getSettingsValue("description1").toString());

    //...
    this->ui->badFramesLineEdit2->setText(taskpipefollowing->getSettingsValue("badFrames2").toString());
    this->ui->camHeightLineEdit2->setText(taskpipefollowing->getSettingsValue("camHeight2").toString());
    this->ui->camWidthLineEdit2->setText(taskpipefollowing->getSettingsValue("camWidth2").toString());
    this->ui->deltaAnglePipeLineEdit2->setText(taskpipefollowing->getSettingsValue("deltaAngle2").toString());
    this->ui->deltaDistPipeLineEdit2->setText(taskpipefollowing->getSettingsValue("deltaDist2").toString());
    this->ui->grayRadioButton2->setChecked(taskpipefollowing->getSettingsValue("convColor2").toInt() == 4);
    this->ui->hRadioButton2->setChecked(taskpipefollowing->getSettingsValue("convColor2").toInt() == 1);
    this->ui->hsvRadioButton2->setChecked(taskpipefollowing->getSettingsValue("convColor2").toInt() == 0);
    this->ui->kpAngleLineEdit2->setText(taskpipefollowing->getSettingsValue("kpAngle2").toString());
    this->ui->kpDistLineEdit2->setText(taskpipefollowing->getSettingsValue("kpDist2").toString());
    this->ui->maxDistLineEdti2->setText(taskpipefollowing->getSettingsValue("maxDistance2").toString());
    this->ui->robCenterXLineEdit2->setText(taskpipefollowing->getSettingsValue("robCenterX2").toString());
    this->ui->robCenterYLineEdit2->setText(taskpipefollowing->getSettingsValue("robCenterY2").toString());
    this->ui->sRadioButton2->setChecked(taskpipefollowing->getSettingsValue("convColor2").toInt() == 2);
    this->ui->speedFwLineEdit2->setText(taskpipefollowing->getSettingsValue("fwSpeed2").toString());
    this->ui->thresholdLineEdit2->setText(taskpipefollowing->getSettingsValue("threshold2").toString());
    this->ui->timer_LineEdit2->setText(taskpipefollowing->getSettingsValue("timer2").toString());
    this->ui->vRadioButton2->setChecked(taskpipefollowing->getSettingsValue("convColor2").toInt() == 3);

    this->ui->checkBox2->setChecked(taskpipefollowing->getSettingsValue("enableUIOutput2").toBool());
    this->ui->frameColorCB2->setChecked(taskpipefollowing->getSettingsValue("frameOutput2").toBool());

    this->ui->durationInput2->setText(this->taskpipefollowing->getSettingsValue("taskDuration2").toString());
    this->ui->descriptionInput2->setText(this->taskpipefollowing->getSettingsValue("description2").toString());

    //...
    this->ui->badFramesLineEdit3->setText(taskpipefollowing->getSettingsValue("badFrames3").toString());
    this->ui->camHeightLineEdit3->setText(taskpipefollowing->getSettingsValue("camHeight3").toString());
    this->ui->camWidthLineEdit3->setText(taskpipefollowing->getSettingsValue("camWidth3").toString());
    this->ui->deltaAnglePipeLineEdit3->setText(taskpipefollowing->getSettingsValue("deltaAngle3").toString());
    this->ui->deltaDistPipeLineEdit3->setText(taskpipefollowing->getSettingsValue("deltaDist3").toString());
    this->ui->grayRadioButton3->setChecked(taskpipefollowing->getSettingsValue("convColor3").toInt() == 4);
    this->ui->hRadioButton3->setChecked(taskpipefollowing->getSettingsValue("convColor3").toInt() == 1);
    this->ui->hsvRadioButton3->setChecked(taskpipefollowing->getSettingsValue("convColor3").toInt() == 0);
    this->ui->kpAngleLineEdit3->setText(taskpipefollowing->getSettingsValue("kpAngle3").toString());
    this->ui->kpDistLineEdit3->setText(taskpipefollowing->getSettingsValue("kpDist3").toString());
    this->ui->maxDistLineEdti3->setText(taskpipefollowing->getSettingsValue("maxDistance3").toString());
    this->ui->robCenterXLineEdit3->setText(taskpipefollowing->getSettingsValue("robCenterX3").toString());
    this->ui->robCenterYLineEdit3->setText(taskpipefollowing->getSettingsValue("robCenterY3").toString());
    this->ui->sRadioButton3->setChecked(taskpipefollowing->getSettingsValue("convColor3").toInt() == 2);
    this->ui->speedFwLineEdit3->setText(taskpipefollowing->getSettingsValue("fwSpeed3").toString());
    this->ui->thresholdLineEdit3->setText(taskpipefollowing->getSettingsValue("threshold3").toString());
    this->ui->timer_LineEdit3->setText(taskpipefollowing->getSettingsValue("timer3").toString());
    this->ui->vRadioButton3->setChecked(taskpipefollowing->getSettingsValue("convColor3").toInt() == 3);

    this->ui->checkBox3->setChecked(taskpipefollowing->getSettingsValue("enableUIOutput3").toBool());
    this->ui->frameColorCB3->setChecked(taskpipefollowing->getSettingsValue("frameOutput3").toBool());

    this->ui->durationInput3->setText(this->taskpipefollowing->getSettingsValue("taskDuration3").toString());
    this->ui->descriptionInput3->setText(this->taskpipefollowing->getSettingsValue("description3").toString());

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
    // Update taskpipefollowing settings

    taskpipefollowing->setSettingsValue("threshold1",ui->thresholdLineEdit1->text().toInt());
    taskpipefollowing->setSettingsValue("timer1",ui->timer_LineEdit1->text().toInt());
    taskpipefollowing->setSettingsValue("deltaDist1",ui->deltaDistPipeLineEdit1->text().toFloat());
    taskpipefollowing->setSettingsValue("deltaAngle1",ui->deltaAnglePipeLineEdit1->text().toFloat());
    taskpipefollowing->setSettingsValue("kpDist1",ui->kpDistLineEdit1->text().toFloat());
    taskpipefollowing->setSettingsValue("kpAngle1",ui->kpAngleLineEdit1->text().toFloat());
    taskpipefollowing->setSettingsValue("robCenterX1",ui->robCenterXLineEdit1->text().toDouble());
    taskpipefollowing->setSettingsValue("robCenterY1",ui->robCenterYLineEdit1->text().toDouble());
    taskpipefollowing->setSettingsValue("maxDistance1",ui->maxDistLineEdti1->text().toFloat());
    taskpipefollowing->setSettingsValue("fwSpeed1",ui->speedFwLineEdit1->text().toFloat());
    taskpipefollowing->setSettingsValue("camHeight1",ui->camHeightLineEdit1->text().toInt());
    taskpipefollowing->setSettingsValue("camWidth1",ui->camWidthLineEdit1->text().toInt());
    taskpipefollowing->setSettingsValue("badFrames1",ui->badFramesLineEdit1->text().toInt());

    taskpipefollowing->setSettingsValue("frameOutput1",ui->frameColorCB1->isChecked());

    if(ui->hRadioButton1->isChecked())
        taskpipefollowing->setSettingsValue("convColor1",1);
    else if(ui->sRadioButton1->isChecked())
        taskpipefollowing->setSettingsValue("convColor1",2);
    else if(ui->vRadioButton1->isChecked())
        taskpipefollowing->setSettingsValue("convColor1",3);
    else if(ui->grayRadioButton1->isChecked())
        taskpipefollowing->setSettingsValue("convColor1",4);
    else if(ui->hsvRadioButton1->isChecked())
        taskpipefollowing->setSettingsValue("convColor1",0);


    if(ui->checkBox1->isChecked())
    {
        //QTimer::singleShot(0,&updateUI,SLOT(start()));
        taskpipefollowing->setSettingsValue("enableUIOutput1",true);
    }
    else if(!ui->checkBox1->isChecked())
        //QTimer::singleShot(0,&updateUI,SLOT(stop()));
        taskpipefollowing->setSettingsValue("enableUIOutput1",false);

    this->taskpipefollowing->setSettingsValue("taskDuration1", this->ui->durationInput1->text());
    this->taskpipefollowing->setSettingsValue("description1", this->ui->descriptionInput1->text());
    emit newSchDesSignal("TaskPipeFollowing1", this->ui->descriptionInput1->text());

    // ...
    taskpipefollowing->setSettingsValue("threshold2",ui->thresholdLineEdit2->text().toInt());
    taskpipefollowing->setSettingsValue("timer2",ui->timer_LineEdit2->text().toInt());
    taskpipefollowing->setSettingsValue("deltaDist2",ui->deltaDistPipeLineEdit2->text().toFloat());
    taskpipefollowing->setSettingsValue("deltaAngle2",ui->deltaAnglePipeLineEdit2->text().toFloat());
    taskpipefollowing->setSettingsValue("kpDist2",ui->kpDistLineEdit2->text().toFloat());
    taskpipefollowing->setSettingsValue("kpAngle2",ui->kpAngleLineEdit2->text().toFloat());
    taskpipefollowing->setSettingsValue("robCenterX2",ui->robCenterXLineEdit2->text().toDouble());
    taskpipefollowing->setSettingsValue("robCenterY2",ui->robCenterYLineEdit2->text().toDouble());
    taskpipefollowing->setSettingsValue("maxDistance2",ui->maxDistLineEdti2->text().toFloat());
    taskpipefollowing->setSettingsValue("fwSpeed2",ui->speedFwLineEdit2->text().toFloat());
    taskpipefollowing->setSettingsValue("camHeight2",ui->camHeightLineEdit2->text().toInt());
    taskpipefollowing->setSettingsValue("camWidth2",ui->camWidthLineEdit2->text().toInt());
    taskpipefollowing->setSettingsValue("badFrames2",ui->badFramesLineEdit2->text().toInt());

    taskpipefollowing->setSettingsValue("frameOutput2",ui->frameColorCB2->isChecked());

    if(ui->hRadioButton2->isChecked())
        taskpipefollowing->setSettingsValue("convColor2",1);
    else if(ui->sRadioButton2->isChecked())
        taskpipefollowing->setSettingsValue("convColor2",2);
    else if(ui->vRadioButton2->isChecked())
        taskpipefollowing->setSettingsValue("convColor2",3);
    else if(ui->grayRadioButton2->isChecked())
        taskpipefollowing->setSettingsValue("convColor2",4);
    else if(ui->hsvRadioButton2->isChecked())
        taskpipefollowing->setSettingsValue("convColor2",0);


    if(ui->checkBox2->isChecked())
    {
        //QTimer::singleShot(0,&updateUI,SLOT(start()));
        taskpipefollowing->setSettingsValue("enableUIOutput2",true);
    }
    else if(!ui->checkBox2->isChecked())
        //QTimer::singleShot(0,&updateUI,SLOT(stop()));
        taskpipefollowing->setSettingsValue("enableUIOutput2",false);

    this->taskpipefollowing->setSettingsValue("taskDuration2", this->ui->durationInput2->text());
    this->taskpipefollowing->setSettingsValue("description2", this->ui->descriptionInput2->text());
    emit newSchDesSignal("TaskPipeFollowing2", this->ui->descriptionInput2->text());

    // ...
    taskpipefollowing->setSettingsValue("threshold3",ui->thresholdLineEdit3->text().toInt());
    taskpipefollowing->setSettingsValue("timer3",ui->timer_LineEdit3->text().toInt());
    taskpipefollowing->setSettingsValue("deltaDist3",ui->deltaDistPipeLineEdit3->text().toFloat());
    taskpipefollowing->setSettingsValue("deltaAngle3",ui->deltaAnglePipeLineEdit3->text().toFloat());
    taskpipefollowing->setSettingsValue("kpDist3",ui->kpDistLineEdit3->text().toFloat());
    taskpipefollowing->setSettingsValue("kpAngle3",ui->kpAngleLineEdit3->text().toFloat());
    taskpipefollowing->setSettingsValue("robCenterX3",ui->robCenterXLineEdit3->text().toDouble());
    taskpipefollowing->setSettingsValue("robCenterY3",ui->robCenterYLineEdit3->text().toDouble());
    taskpipefollowing->setSettingsValue("maxDistance3",ui->maxDistLineEdti3->text().toFloat());
    taskpipefollowing->setSettingsValue("fwSpeed3",ui->speedFwLineEdit3->text().toFloat());
    taskpipefollowing->setSettingsValue("camHeight3",ui->camHeightLineEdit3->text().toInt());
    taskpipefollowing->setSettingsValue("camWidth3",ui->camWidthLineEdit3->text().toInt());
    taskpipefollowing->setSettingsValue("badFrames3",ui->badFramesLineEdit3->text().toInt());

    taskpipefollowing->setSettingsValue("frameOutput3",ui->frameColorCB3->isChecked());

    if(ui->hRadioButton3->isChecked())
        taskpipefollowing->setSettingsValue("convColor3",1);
    else if(ui->sRadioButton3->isChecked())
        taskpipefollowing->setSettingsValue("convColor3",2);
    else if(ui->vRadioButton3->isChecked())
        taskpipefollowing->setSettingsValue("convColor3",3);
    else if(ui->grayRadioButton3->isChecked())
        taskpipefollowing->setSettingsValue("convColor3",4);
    else if(ui->hsvRadioButton3->isChecked())
        taskpipefollowing->setSettingsValue("convColor3",0);


    if(ui->checkBox3->isChecked())
    {
        //QTimer::singleShot(0,&updateUI,SLOT(start()));
        taskpipefollowing->setSettingsValue("enableUIOutput3",true);
    }
    else if(!ui->checkBox3->isChecked())
        //QTimer::singleShot(0,&updateUI,SLOT(stop()));
        taskpipefollowing->setSettingsValue("enableUIOutput3",false);

    this->taskpipefollowing->setSettingsValue("taskDuration3", this->ui->durationInput3->text());
    this->taskpipefollowing->setSettingsValue("description3", this->ui->descriptionInput3->text());
    emit newSchDesSignal("TaskPipeFollowing3", this->ui->descriptionInput3->text());
}
