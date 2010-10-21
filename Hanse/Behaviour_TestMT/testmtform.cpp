#include "testmtform.h"
#include "ui_testmtform.h"
#include <QFileDialog>

TestMTForm::TestMTForm(QWidget *parent, Behaviour_TestMT *testmt) :
    QWidget(parent),
    ui(new Ui::TestMTForm)
{
    ui->setupUi(this);
    QObject::connect(this,SIGNAL(startBehaviour()),testmt,SLOT(start()));
    QObject::connect(this,SIGNAL(stopBehaviour()),testmt,SLOT(stop()));

 }

TestMTForm::~TestMTForm()
{
    delete ui;
}

void TestMTForm::changeEvent(QEvent *e)
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

void TestMTForm::on_startButton_clicked()
{
    emit startBehaviour();
}

void TestMTForm::on_stopButton_clicked()
{
    emit stopBehaviour();
}
