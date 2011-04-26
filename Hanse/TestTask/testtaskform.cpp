#include "testtaskform.h"
#include "ui_testtaskform.h"

TestTaskForm::TestTaskForm(TestTask *tt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestTaskForm)
{
    ui->setupUi(this);
    this->testtask = tt;
}

TestTaskForm::~TestTaskForm()
{
    delete ui;
}

void TestTaskForm::changeEvent(QEvent *e)
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

void TestTaskForm::on_startButton_clicked(){
    qDebug("startButtonClicked");
}

void TestTaskForm::on_stopButton_clicked(){
    qDebug("stopButtonClicked");
}
