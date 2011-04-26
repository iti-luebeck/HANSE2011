#include "testtask2form.h"
#include "ui_testtask2form.h"

TestTask2Form::TestTask2Form(TestTask2 *tt, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestTask2Form)
{
    ui->setupUi(this);
    this->testtask2 = tt;
}

TestTask2Form::~TestTask2Form()
{
    delete ui;
}

void TestTask2Form::changeEvent(QEvent *e)
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

void TestTask2Form::on_startButton_clicked(){
    qDebug("startButtonClicked");
}

void TestTask2Form::on_stopButton_clicked(){
    qDebug("stopButtonClicked");
}
