#include "taskwallfollowingform.h"
#include "ui_taskwallfollowingform.h"

TaskWallFollowingForm::TaskWallFollowingForm(TaskWallFollowing *tw, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskWallFollowingForm)
{
    ui->setupUi(this);
    this->taskwallfollowing = tw;

    connect(taskwallfollowing,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    connect(taskwallfollowing,SIGNAL(setDescriptionSignal()),this,SLOT(returnDescription()));
    connect(this,SIGNAL(newSchDesSignal(QString, QString)),taskwallfollowing,SLOT(newSchDesSlot(QString, QString)));


    // Show settings from taskwallfollowing
    this->ui->forwardInput1->setText(this->taskwallfollowing->getSettingsValue("forwardSpeed1").toString());
    this->ui->angularInput1->setText(this->taskwallfollowing->getSettingsValue("angularSpeed1").toString());
    this->ui->desiredDistanceInput1->setText(this->taskwallfollowing->getSettingsValue("desiredDistance1").toString());
    this->ui->corridorInput1->setText(this->taskwallfollowing->getSettingsValue("corridorWidth1").toString());
    this->ui->durationInput1->setText(this->taskwallfollowing->getSettingsValue("taskDuration1").toString());
    this->ui->descriptionInput1->setText(this->taskwallfollowing->getSettingsValue("description1").toString());

    this->ui->forwardInput2->setText(this->taskwallfollowing->getSettingsValue("forwardSpeed2").toString());
    this->ui->angularInput2->setText(this->taskwallfollowing->getSettingsValue("angularSpeed2").toString());
    this->ui->desiredDistanceInput2->setText(this->taskwallfollowing->getSettingsValue("desiredDistance2").toString());
    this->ui->corridorInput2->setText(this->taskwallfollowing->getSettingsValue("corridorWidth2").toString());
    this->ui->durationInput2->setText(this->taskwallfollowing->getSettingsValue("taskDuration2").toString());
    this->ui->descriptionInput2->setText(this->taskwallfollowing->getSettingsValue("description2").toString());

    this->ui->forwardInput3->setText(this->taskwallfollowing->getSettingsValue("forwardSpeed3").toString());
    this->ui->angularInput3->setText(this->taskwallfollowing->getSettingsValue("angularSpeed3").toString());
    this->ui->desiredDistanceInput3->setText(this->taskwallfollowing->getSettingsValue("desiredDistance3").toString());
    this->ui->corridorInput3->setText(this->taskwallfollowing->getSettingsValue("corridorWidth3").toString());
    this->ui->durationInput3->setText(this->taskwallfollowing->getSettingsValue("taskDuration3").toString());
    this->ui->descriptionInput3->setText(this->taskwallfollowing->getSettingsValue("description3").toString());

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
    this->taskwallfollowing->setSettingsValue("forwardSpeed1" ,this->ui->forwardInput1->text());
    this->taskwallfollowing->setSettingsValue("angularSpeed1" ,this->ui->angularInput1->text());
    this->taskwallfollowing->setSettingsValue("desiredDistance1" ,this->ui->desiredDistanceInput1->text());
    this->taskwallfollowing->setSettingsValue("corridorWidth1" ,this->ui->corridorInput1->text());
    this->taskwallfollowing->setSettingsValue("taskDuration1", this->ui->durationInput1->text());
    this->taskwallfollowing->setSettingsValue("description1", this->ui->descriptionInput1->text());
    emit newSchDesSignal("Wall1", this->ui->descriptionInput1->text());

    this->taskwallfollowing->setSettingsValue("forwardSpeed2" ,this->ui->forwardInput2->text());
    this->taskwallfollowing->setSettingsValue("angularSpeed2" ,this->ui->angularInput2->text());
    this->taskwallfollowing->setSettingsValue("desiredDistance2" ,this->ui->desiredDistanceInput2->text());
    this->taskwallfollowing->setSettingsValue("corridorWidth2" ,this->ui->corridorInput2->text());
    this->taskwallfollowing->setSettingsValue("taskDuration2", this->ui->durationInput2->text());
    this->taskwallfollowing->setSettingsValue("description2", this->ui->descriptionInput2->text());
    emit newSchDesSignal("Wall2", this->ui->descriptionInput2->text());

    this->taskwallfollowing->setSettingsValue("forwardSpeed3" ,this->ui->forwardInput3->text());
    this->taskwallfollowing->setSettingsValue("angularSpeed3" ,this->ui->angularInput3->text());
    this->taskwallfollowing->setSettingsValue("desiredDistance3" ,this->ui->desiredDistanceInput3->text());
    this->taskwallfollowing->setSettingsValue("corridorWidth3" ,this->ui->corridorInput3->text());
    this->taskwallfollowing->setSettingsValue("taskDuration3", this->ui->durationInput3->text());
    this->taskwallfollowing->setSettingsValue("description3", this->ui->descriptionInput3->text());
    emit newSchDesSignal("Wall3", this->ui->descriptionInput3->text());
}

void TaskWallFollowingForm::returnDescription(){
    emit newSchDesSignal("Wall1", this->ui->descriptionInput1->text());
    emit newSchDesSignal("Wall2", this->ui->descriptionInput2->text());
    emit newSchDesSignal("Wall3", this->ui->descriptionInput3->text());
}
