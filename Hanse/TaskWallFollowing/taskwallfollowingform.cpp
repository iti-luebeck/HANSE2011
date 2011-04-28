#include "taskwallfollowingform.h"
#include "ui_taskwallfollowingform.h"

TaskWallFollowingForm::TaskWallFollowingForm(TaskWallFollowing *tw, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskWallFollowingForm)
{
    ui->setupUi(this);
    this->taskwallfollowing = tw;

    connect(taskwallfollowing,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));

    // Show the default settings from taskwallfollowing
    this->ui->forwardInput->setText(this->taskwallfollowing->getSettingsValue("forwardSpeed").toString());
    this->ui->angularInput->setText(this->taskwallfollowing->getSettingsValue("angularSpeed").toString());
    this->ui->desiredDistanceInput->setText(this->taskwallfollowing->getSettingsValue("desiredDistance").toString());
    this->ui->corridorInput->setText(this->taskwallfollowing->getSettingsValue("corridorWidth").toString());
    this->ui->durationInput->setText(this->taskwallfollowing->getSettingsValue("taskDuration").toString());
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
    // Update taskwallfollowing settings
    this->taskwallfollowing->setSettingsValue("forwardSpeed" ,this->ui->forwardInput->text());
    this->taskwallfollowing->setSettingsValue("angularSpeed" ,this->ui->angularInput->text());
    this->taskwallfollowing->setSettingsValue("desiredDistance" ,this->ui->desiredDistanceInput->text());
    this->taskwallfollowing->setSettingsValue("corridorWidth" ,this->ui->corridorInput->text());
    this->taskwallfollowing->setSettingsValue("taskDuration", this->ui->durationInput->text());
}

