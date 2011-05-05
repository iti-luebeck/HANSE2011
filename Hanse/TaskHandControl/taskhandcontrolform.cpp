#include "taskhandcontrolform.h"
#include "ui_taskhandcontrolform.h"

TaskHandControlForm::TaskHandControlForm(TaskHandControl *thc, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskHandControlForm)
{
    ui->setupUi(this);
    this->taskhandcontrol = thc;

    connect(taskhandcontrol,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    connect(this,SIGNAL(handControlFinishedSignal()),taskhandcontrol,SLOT(handControlFinishedSlot()));

    // Show settings from taskhandcontrol

}

TaskHandControlForm::~TaskHandControlForm()
{
    delete ui;
}

void TaskHandControlForm::changeEvent(QEvent *e)
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

void TaskHandControlForm::on_applyButton_clicked(){
    emit handControlFinishedSignal();
}


