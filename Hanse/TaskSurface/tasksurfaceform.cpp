#include "tasksurfaceform.h"
#include "ui_tasksurfaceform.h"

TaskSurfaceForm::TaskSurfaceForm(TaskSurface *tf, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskSurfaceForm)
{
    ui->setupUi(this);
    this->tasksurface = tf;

    connect(tasksurface,SIGNAL(updateSettings()),this,SLOT(on_applyButton_clicked()));

}

TaskSurfaceForm::~TaskSurfaceForm()
{
    delete ui;
}

void TaskSurfaceForm::changeEvent(QEvent *e)
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

void TaskSurfaceForm::on_applyButton_clicked(){

}

