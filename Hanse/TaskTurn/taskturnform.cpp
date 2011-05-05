#include "taskturnform.h"
#include "ui_taskturnform.h"

TaskTurnForm::TaskTurnForm(TaskTurn *tt, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskTurnForm)
{
    ui->setupUi(this);
    this->taskturn = tt;

    connect(taskturn,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    connect(taskturn,SIGNAL(setDescriptionSignal()),this,SLOT(on_applyButton_clicked()));
    connect(this,SIGNAL(newSchDesSignal(QString, QString)),taskturn,SLOT(newSchDesSlot(QString, QString)));

    // Show settings from taskthrustercontrol
//    this->ui->forwardInput1->setText(this->taskthrustercontrol->getSettingsValue("forwardSpeed1").toString());
//    this->ui->angularInput1->setText(this->taskthrustercontrol->getSettingsValue("angularSpeed1").toString());
//    this->ui->desiredDepthInput1->setText(this->taskthrustercontrol->getSettingsValue("desiredDepth1").toString());
//    this->ui->durationInput1->setText(this->taskthrustercontrol->getSettingsValue("taskDuration1").toString());
//    this->ui->descriptionInput1->setText(this->taskthrustercontrol->getSettingsValue("description1").toString());

 }

TaskTurnForm::~TaskTurnForm()
{
    delete ui;
}

void TaskTurnForm::changeEvent(QEvent *e)
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

void TaskTurnForm::on_applyButton_clicked(){
    // Update taskturn settings
//    this->taskthrustercontrol->setSettingsValue("forwardSpeed1" ,this->ui->forwardInput1->text());
//    this->taskthrustercontrol->setSettingsValue("angularSpeed1" ,this->ui->angularInput1->text());
//    this->taskthrustercontrol->setSettingsValue("desiredDepth1" ,this->ui->desiredDepthInput1->text());
//    this->taskthrustercontrol->setSettingsValue("taskDuration1", this->ui->durationInput1->text());
//    this->taskthrustercontrol->setSettingsValue("description1", this->ui->descriptionInput1->text());
//    emit newSchDesSignal("TaskThrusterControl1", this->ui->descriptionInput1->text());



}

