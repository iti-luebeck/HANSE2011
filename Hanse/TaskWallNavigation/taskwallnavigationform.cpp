#include "taskwallnavigationform.h"
#include "ui_taskwallnavigationform.h"

TaskWallNavigationForm::TaskWallNavigationForm(TaskWallNavigation *twn, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskWallNavigationForm)
{
    ui->setupUi(this);
    this->taskwallnavigation = twn;

    connect(taskwallnavigation,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    connect(taskwallnavigation,SIGNAL(setDescriptionSignal()),this,SLOT(returnDescription()));
    connect(this,SIGNAL(newSchDesSignal(QString, QString)),taskwallnavigation,SLOT(newSchDesSlot(QString, QString)));
    connect(this, SIGNAL(updateTaskSettingsSignal()), taskwallnavigation, SLOT(updateTaskSettingsSlot()));

    // Show settings from taskwallfollowing
    this->ui->forwardInput->setText(this->taskwallnavigation->getSettingsValue("forwardSpeed").toString());
    this->ui->angularInput->setText(this->taskwallnavigation->getSettingsValue("angularSpeed").toString());
    this->ui->desiredDistanceInput->setText(this->taskwallnavigation->getSettingsValue("desiredDistance").toString());
    this->ui->corridorInput->setText(this->taskwallnavigation->getSettingsValue("corridorWidth").toString());
    this->ui->durationInput->setText(this->taskwallnavigation->getSettingsValue("taskDuration").toString());
    this->ui->descriptionInput->setText(this->taskwallnavigation->getSettingsValue("description").toString());

}

TaskWallNavigationForm::~TaskWallFollowingForm()
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
    this->taskwallnavigation->setSettingsValue("description", this->ui->descriptionInput->text());
    this->taskwallnavigation->setSettingsValue("navigationTarget", this->ui->startInput->text());
    this->taskwallnavigation->setSettingsValue("navigationStart", this->ui->targetInput->text());
    emit newSchDesSignal("Wall1", this->ui->descriptionInput1->text());


}


