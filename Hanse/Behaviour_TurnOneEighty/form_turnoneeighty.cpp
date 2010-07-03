#include "form_turnoneeighty.h"
#include "ui_form_turnoneeighty.h"
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>

Form_TurnOneEighty::Form_TurnOneEighty( Behaviour_TurnOneEighty *behaviour, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_TurnOneEighty)
{
    ui->setupUi(this);
    this->behaviour = behaviour;

    QSettings& settings = behaviour->getSettings();
    ui->pEdit->setText( settings.value( "p", TURN_DEFAULT_P ).toString() );
    ui->hysteresisEdit->setText( settings.value( "hysteresis", TURN_DEFAULT_HYSTERESIS ).toString() );
}

Form_TurnOneEighty::~Form_TurnOneEighty()
{
    delete ui;
}

void Form_TurnOneEighty::changeEvent(QEvent *e)
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

void Form_TurnOneEighty::on_applyButton_clicked()
{
    QSettings& settings = behaviour->getSettings();
    settings.setValue( "p", ui->pEdit->text().toDouble() );
    settings.setValue( "hysteresis", ui->hysteresisEdit->text().toDouble() );
}

void Form_TurnOneEighty::on_startButton_clicked()
{
    behaviour->start();
}

void Form_TurnOneEighty::on_stopButton_clicked()
{
    behaviour->stop();
}
