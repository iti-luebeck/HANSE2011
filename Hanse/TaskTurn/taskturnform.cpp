#include "taskturnform.h"
#include "ui_taskturnform.h"

TaskTurnForm::TaskTurnForm(TaskTurn *tt, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskTurnForm)
{
    ui->setupUi(this);
    this->taskturn = tt;

    connect(taskturn,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    connect(taskturn,SIGNAL(setDescriptionSignal()),this,SLOT(returnDescription()));
    connect(this,SIGNAL(newSchDesSignal(QString, QString)),taskturn,SLOT(newSchDesSlot(QString, QString)));

    // Show settings from taskthrustercontrol
    this->ui->forwardInput1->setText(this->taskturn->getSettingsValue("forwardSpeed1").toString());
    this->ui->angularInput1->setText(this->taskturn->getSettingsValue("angularSpeed1").toString());
    this->ui->degreeInput1->setText(this->taskturn->getSettingsValue("degree1").toString());
    this->ui->toleranceInput1->setText(this->taskturn->getSettingsValue("tolerance1").toString());
    this->ui->updateInput1->setText(this->taskturn->getSettingsValue("update1").toString());
    this->ui->descriptionInput1->setText(this->taskturn->getSettingsValue("description1").toString());
    this->ui->pInput1->setText(this->taskturn->getSettingsValue("p1").toString());

    this->ui->forwardInput2->setText(this->taskturn->getSettingsValue("forwardSpeed2").toString());
    this->ui->angularInput2->setText(this->taskturn->getSettingsValue("angularSpeed2").toString());
    this->ui->degreeInput2->setText(this->taskturn->getSettingsValue("degree2").toString());
    this->ui->toleranceInput2->setText(this->taskturn->getSettingsValue("tolerance2").toString());
    this->ui->updateInput2->setText(this->taskturn->getSettingsValue("update2").toString());
    this->ui->descriptionInput2->setText(this->taskturn->getSettingsValue("description2").toString());
    this->ui->pInput2->setText(this->taskturn->getSettingsValue("p2").toString());

    this->ui->forwardInput3->setText(this->taskturn->getSettingsValue("forwardSpeed3").toString());
    this->ui->angularInput3->setText(this->taskturn->getSettingsValue("angularSpeed3").toString());
    this->ui->degreeInput3->setText(this->taskturn->getSettingsValue("degree3").toString());
    this->ui->toleranceInput3->setText(this->taskturn->getSettingsValue("tolerance3").toString());
    this->ui->updateInput3->setText(this->taskturn->getSettingsValue("update3").toString());
    this->ui->descriptionInput3->setText(this->taskturn->getSettingsValue("description3").toString());
    this->ui->pInput3->setText(this->taskturn->getSettingsValue("p3").toString());

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
    this->taskturn->setSettingsValue("degree1" ,this->ui->degreeInput1->text());
    this->taskturn->setSettingsValue("tolerance1" ,this->ui->toleranceInput1->text());
    this->taskturn->setSettingsValue("update1" ,this->ui->updateInput1->text());
    this->taskturn->setSettingsValue("forwardSpeed1" ,this->ui->forwardInput1->text());
    this->taskturn->setSettingsValue("angularSpeed1" ,this->ui->angularInput1->text());
    this->taskturn->setSettingsValue("description1", this->ui->descriptionInput1->text());
    this->taskturn->setSettingsValue("p1", this->ui->descriptionInput1->text());
    emit newSchDesSignal("Turn1", this->ui->descriptionInput1->text());

    this->taskturn->setSettingsValue("degree2" ,this->ui->degreeInput2->text());
    this->taskturn->setSettingsValue("tolerance2" ,this->ui->toleranceInput2->text());
    this->taskturn->setSettingsValue("update2" ,this->ui->updateInput2->text());
    this->taskturn->setSettingsValue("forwardSpeed2" ,this->ui->forwardInput2->text());
    this->taskturn->setSettingsValue("angularSpeed2" ,this->ui->angularInput2->text());
    this->taskturn->setSettingsValue("description2", this->ui->descriptionInput2->text());
    this->taskturn->setSettingsValue("p2", this->ui->descriptionInput2->text());
    emit newSchDesSignal("Turn2", this->ui->descriptionInput2->text());

    this->taskturn->setSettingsValue("degree3" ,this->ui->degreeInput3->text());
    this->taskturn->setSettingsValue("tolerance3" ,this->ui->toleranceInput3->text());
    this->taskturn->setSettingsValue("update3" ,this->ui->updateInput3->text());
    this->taskturn->setSettingsValue("forwardSpeed3" ,this->ui->forwardInput3->text());
    this->taskturn->setSettingsValue("angularSpeed3" ,this->ui->angularInput3->text());
    this->taskturn->setSettingsValue("description3", this->ui->descriptionInput3->text());
    this->taskturn->setSettingsValue("p3", this->ui->descriptionInput3->text());
    emit newSchDesSignal("Turn3", this->ui->descriptionInput3->text());
}

void TaskTurnForm::returnDescription(){
    emit newSchDesSignal("Turn1", this->ui->descriptionInput1->text());
    emit newSchDesSignal("Turn2", this->ui->descriptionInput2->text());
    emit newSchDesSignal("Turn3", this->ui->descriptionInput3->text());

}
