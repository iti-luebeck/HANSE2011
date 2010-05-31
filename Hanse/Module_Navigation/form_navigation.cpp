#include "form_navigation.h"
#include "ui_form_navigation.h"

Form_Navigation::Form_Navigation(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_Navigation)
{
    ui->setupUi(this);
}

Form_Navigation::~Form_Navigation()
{
    delete ui;
}

void Form_Navigation::changeEvent(QEvent *e)
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
