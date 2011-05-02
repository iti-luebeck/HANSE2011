#include "taskthrustercontrolform.h"
#include "ui_taskthrustercontrolform.h"

TaskThrusterControlForm::TaskThrusterControlForm(TaskThrusterControl *ttc, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskThrusterControlForm)
{
    ui->setupUi(this);
    this->taskthrustercontrol = ttc;

    connect(taskthrustercontrol,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    connect(taskthrustercontrol,SIGNAL(setDescriptionSignal()),this,SLOT(on_applyButton_clicked()));
    connect(this,SIGNAL(newSchDesSignal(QString, QString)),taskthrustercontrol,SLOT(newSchDesSlot(QString, QString)));

    // Show settings from taskthrustercontrol
    this->ui->forwardInput1->setText(this->taskthrustercontrol->getSettingsValue("forwardSpeed1").toString());
    this->ui->angularInput1->setText(this->taskthrustercontrol->getSettingsValue("angularSpeed1").toString());
    this->ui->desiredDepthInput1->setText(this->taskthrustercontrol->getSettingsValue("desiredDepth1").toString());
    this->ui->durationInput1->setText(this->taskthrustercontrol->getSettingsValue("taskDuration1").toString());
    this->ui->descriptionInput1->setText(this->taskthrustercontrol->getSettingsValue("description1").toString());

    this->ui->forwardInput2->setText(this->taskthrustercontrol->getSettingsValue("forwardSpeed2").toString());
    this->ui->angularInput2->setText(this->taskthrustercontrol->getSettingsValue("angularSpeed2").toString());
    this->ui->desiredDepthInput2->setText(this->taskthrustercontrol->getSettingsValue("desiredDepth2").toString());
    this->ui->durationInput2->setText(this->taskthrustercontrol->getSettingsValue("taskDuration2").toString());
    this->ui->descriptionInput2->setText(this->taskthrustercontrol->getSettingsValue("description2").toString());

    this->ui->forwardInput3->setText(this->taskthrustercontrol->getSettingsValue("forwardSpeed3").toString());
    this->ui->angularInput3->setText(this->taskthrustercontrol->getSettingsValue("angularSpeed3").toString());
    this->ui->desiredDepthInput3->setText(this->taskthrustercontrol->getSettingsValue("desiredDepth3").toString());
    this->ui->durationInput3->setText(this->taskthrustercontrol->getSettingsValue("taskDuration3").toString());
    this->ui->descriptionInput3->setText(this->taskthrustercontrol->getSettingsValue("description3").toString());

    this->ui->forwardInput4->setText(this->taskthrustercontrol->getSettingsValue("forwardSpeed4").toString());
    this->ui->angularInput4->setText(this->taskthrustercontrol->getSettingsValue("angularSpeed4").toString());
    this->ui->desiredDepthInput4->setText(this->taskthrustercontrol->getSettingsValue("desiredDepth4").toString());
    this->ui->durationInput4->setText(this->taskthrustercontrol->getSettingsValue("taskDuration4").toString());
    this->ui->descriptionInput4->setText(this->taskthrustercontrol->getSettingsValue("description4").toString());

    this->ui->forwardInput5->setText(this->taskthrustercontrol->getSettingsValue("forwardSpeed5").toString());
    this->ui->angularInput5->setText(this->taskthrustercontrol->getSettingsValue("angularSpeed5").toString());
    this->ui->desiredDepthInput5->setText(this->taskthrustercontrol->getSettingsValue("desiredDepth5").toString());
    this->ui->durationInput5->setText(this->taskthrustercontrol->getSettingsValue("taskDuration5").toString());
    this->ui->descriptionInput5->setText(this->taskthrustercontrol->getSettingsValue("description5").toString());

    this->ui->forwardInput6->setText(this->taskthrustercontrol->getSettingsValue("forwardSpeed6").toString());
    this->ui->angularInput6->setText(this->taskthrustercontrol->getSettingsValue("angularSpeed6").toString());
    this->ui->desiredDepthInput6->setText(this->taskthrustercontrol->getSettingsValue("desiredDepth6").toString());
    this->ui->durationInput6->setText(this->taskthrustercontrol->getSettingsValue("taskDuration6").toString());
    this->ui->descriptionInput6->setText(this->taskthrustercontrol->getSettingsValue("description6").toString());
}

TaskThrusterControlForm::~TaskThrusterControlForm()
{
    delete ui;
}

void TaskThrusterControlForm::changeEvent(QEvent *e)
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

void TaskThrusterControlForm::on_applyButton_clicked(){
    // Update taskthrustercontrol settings
    this->taskthrustercontrol->setSettingsValue("forwardSpeed1" ,this->ui->forwardInput1->text());
    this->taskthrustercontrol->setSettingsValue("angularSpeed1" ,this->ui->angularInput1->text());
    this->taskthrustercontrol->setSettingsValue("desiredDepth1" ,this->ui->desiredDepthInput1->text());
    this->taskthrustercontrol->setSettingsValue("taskDuration1", this->ui->durationInput1->text());
    this->taskthrustercontrol->setSettingsValue("description1", this->ui->descriptionInput1->text());
    emit newSchDesSignal("TaskThrusterControl1", this->ui->descriptionInput1->text());

    this->taskthrustercontrol->setSettingsValue("forwardSpeed2" ,this->ui->forwardInput2->text());
    this->taskthrustercontrol->setSettingsValue("angularSpeed2" ,this->ui->angularInput2->text());
    this->taskthrustercontrol->setSettingsValue("desiredDepth2" ,this->ui->desiredDepthInput2->text());
    this->taskthrustercontrol->setSettingsValue("taskDuration2", this->ui->durationInput2->text());
    this->taskthrustercontrol->setSettingsValue("description2", this->ui->descriptionInput2->text());
    emit newSchDesSignal("TaskThrusterControl2", this->ui->descriptionInput2->text());

    this->taskthrustercontrol->setSettingsValue("forwardSpeed3" ,this->ui->forwardInput3->text());
    this->taskthrustercontrol->setSettingsValue("angularSpeed3" ,this->ui->angularInput3->text());
    this->taskthrustercontrol->setSettingsValue("desiredDepth3" ,this->ui->desiredDepthInput3->text());
    this->taskthrustercontrol->setSettingsValue("taskDuration3", this->ui->durationInput3->text());
    this->taskthrustercontrol->setSettingsValue("description3", this->ui->descriptionInput3->text());
    emit newSchDesSignal("TaskThrusterControl3", this->ui->descriptionInput3->text());

    this->taskthrustercontrol->setSettingsValue("forwardSpeed4" ,this->ui->forwardInput4->text());
    this->taskthrustercontrol->setSettingsValue("angularSpeed4" ,this->ui->angularInput4->text());
    this->taskthrustercontrol->setSettingsValue("desiredDepth4" ,this->ui->desiredDepthInput4->text());
    this->taskthrustercontrol->setSettingsValue("taskDuration4", this->ui->durationInput4->text());
    this->taskthrustercontrol->setSettingsValue("description4", this->ui->descriptionInput4->text());
    emit newSchDesSignal("TaskThrusterControl4", this->ui->descriptionInput4->text());

    this->taskthrustercontrol->setSettingsValue("forwardSpeed5" ,this->ui->forwardInput5->text());
    this->taskthrustercontrol->setSettingsValue("angularSpeed5" ,this->ui->angularInput5->text());
    this->taskthrustercontrol->setSettingsValue("desiredDepth5" ,this->ui->desiredDepthInput5->text());
    this->taskthrustercontrol->setSettingsValue("taskDuration5", this->ui->durationInput5->text());
    this->taskthrustercontrol->setSettingsValue("description5", this->ui->descriptionInput5->text());
    emit newSchDesSignal("TaskThrusterControl5", this->ui->descriptionInput5->text());

    this->taskthrustercontrol->setSettingsValue("forwardSpeed6" ,this->ui->forwardInput6->text());
    this->taskthrustercontrol->setSettingsValue("angularSpeed6" ,this->ui->angularInput6->text());
    this->taskthrustercontrol->setSettingsValue("desiredDepth6" ,this->ui->desiredDepthInput6->text());
    this->taskthrustercontrol->setSettingsValue("taskDuration6", this->ui->durationInput6->text());
    this->taskthrustercontrol->setSettingsValue("description6", this->ui->descriptionInput6->text());
    emit newSchDesSignal("TaskThrusterControl6", this->ui->descriptionInput6->text());
}

