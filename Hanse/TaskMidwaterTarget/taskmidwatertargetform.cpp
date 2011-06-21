#include "taskmidwatertargetform.h"
#include "ui_taskmidwatertargetform.h"

TaskMidwaterTargetForm::TaskMidwaterTargetForm(TaskMidwaterTarget *tmt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskMidwaterTargetForm)
{
    ui->setupUi(this);
    this->taskmidwatertarget = tmt;

    connect(taskmidwatertarget,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

}

TaskMidwaterTargetForm::~TaskMidwaterTargetForm()
{
    delete ui;
}

void TaskMidwaterTargetForm::changeEvent(QEvent *e)
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

void TaskMidwaterTargetForm::on_applyButton_clicked(){

}

