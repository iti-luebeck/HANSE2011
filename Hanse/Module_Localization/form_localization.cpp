#include "form_localization.h"
#include "ui_form_localization.h"

Form_Localization::Form_Localization(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form_Localization)
{
    ui->setupUi(this);
}

Form_Localization::~Form_Localization()
{
    delete ui;
}

void Form_Localization::changeEvent(QEvent *e)
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
