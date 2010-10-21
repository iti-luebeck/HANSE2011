#include "testmtform.h"
#include "ui_testmtform.h"
#include <QFileDialog>

TestMTForm::TestMTForm(QWidget *parent, Behaviour_TestMT *TestMT) :
    QWidget(parent),
    ui(new Ui::TestMTForm)
{
    ui->setupUi(this);
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
