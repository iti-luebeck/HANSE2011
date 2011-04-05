#include "commandcenterform.h"
#include "ui_commandcenterform.h"

CommandCenterForm::CommandCenterForm(CommandCenter *commandcenter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandCenterForm)
{
    ui->setupUi(this);
    this->com = commandcenter;
    schedule = "";

    QObject::connect(this,SIGNAL(newSchedule()),com,SLOT(newSchedule()));

}

CommandCenterForm::~CommandCenterForm()
{
    delete ui;
}

void CommandCenterForm::changeEvent(QEvent *e)
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


void CommandCenterForm::on_addButton_clicked(){
   // ui->scheduleList->setText(ui->scheduleInput->currentText());
    qDebug("addButtonClicked");
    ui->scheduleList->insertPlainText(ui->scheduleInput->currentText()+"\n");
    schedule = schedule+ui->scheduleInput->currentText()+",";
    qDebug()<<schedule;
}

void CommandCenterForm::on_revertButton_clicked(){
    // readOnly must be switched off
    ui->scheduleList->cut();
}

void CommandCenterForm::on_clearButton_clicked(){
   // ui->scheduleList->setText(ui->scheduleInput->currentText());
    qDebug("clearButtonClicked");
    ui->scheduleList->clear();
    schedule = "";
}

void CommandCenterForm::on_startButton_clicked(){
    qDebug("startButtonClicked");
    emit newSchedule();
}

void CommandCenterForm::on_stopButton_clicked(){
    qDebug("stopButtonClicked");

}
