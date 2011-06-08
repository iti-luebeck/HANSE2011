#include "form_turnoneeighty.h"
#include "ui_form_turnoneeighty.h"
#include <Behaviour_TurnOneEighty/behaviour_turnoneeighty.h>

Form_TurnOneEighty::Form_TurnOneEighty( Behaviour_TurnOneEighty *behaviour, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_TurnOneEighty)
{
    ui->setupUi(this);
    this->behaviour = behaviour;

//    QSettings& settings = behaviour->getSettings();
    ui->pEdit->setText( behaviour->getSettingsValue( "p", TURN_DEFAULT_P ).toString() );
    ui->hysteresisEdit->setText( behaviour->getSettingsValue( "hysteresis", TURN_DEFAULT_HYSTERESIS ).toString() );
    ui->degreeInput->setText(behaviour->getSettingsValue("degree").toString());
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
//    QSettings& settings = behaviour->getSettings();
    behaviour->setSettingsValue( "p", ui->pEdit->text().toDouble() );
    behaviour->setSettingsValue( "hysteresis", ui->hysteresisEdit->text().toDouble() );
    behaviour->setSettingsValue("degree", ui->degreeInput->text().toDouble());
}

void Form_TurnOneEighty::on_startButton_clicked()
{

    behaviour->setSettingsValue( "p", ui->pEdit->text().toDouble() );
    behaviour->setSettingsValue( "hysteresis", ui->hysteresisEdit->text().toDouble() );
    behaviour->setSettingsValue("degree", ui->degreeInput->text().toDouble());
    behaviour->startBehaviour();
}

void Form_TurnOneEighty::on_stopButton_clicked()
{
    behaviour->stop();
}
