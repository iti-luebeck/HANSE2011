#include "form.h"
#include "ui_form.h"

Form::Form(Module_ScanningSonar* sonar, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    this->sonar = sonar;
    connect(ui->pushButton, SIGNAL(clicked()), sonar, SLOT(doNextScan()));
}

Form::~Form()
{
    delete ui;
}

void Form::changeEvent(QEvent *e)
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

void Form::on_pushButton_clicked()
{
}
