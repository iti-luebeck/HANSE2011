#include "commandcenterform.h"
#include "ui_commandcenterform.h"

CommandCenterForm::CommandCenterForm(CommandCenter *commandcenter, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandCenterForm)
{

    this->com = commandcenter;

     //qRegisterMetaType<QList<QString> >("QList<QString>");
     QObject::connect(this,SIGNAL(startCommandCenter()),com,SLOT(startCC()));
   // QObject::connect(this,SIGNAL(startCommandCenter(QList<QString>)),com,SLOT(startCommandCenter(QList<QString>)));
     connect(this,SIGNAL(stopCommandCenter()),com,SLOT(stopCC()));

    connect(com,SIGNAL(nData(QString)),this,SLOT(updateCcUI(QString)));

    ui->setupUi(this);
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
    this->com->schedule.prepend(ui->scheduleInput->currentText());
    qDebug()<<ui->scheduleInput->currentText();
}

void CommandCenterForm::on_revertButton_clicked(){
    // readOnly must be switched off
    if(!this->com->schedule.isEmpty()){
        this->com->schedule.removeFirst();
    }
    ui->scheduleList->clear();
    for(int i = this->com->schedule.length()-1; i>=0; i--){
        ui->scheduleList->insertPlainText(this->com->schedule.at(i)+"\n");
    }
}

void CommandCenterForm::on_clearButton_clicked(){
   // ui->scheduleList->setText(ui->scheduleInput->currentText());
    qDebug("clearButtonClicked");
    ui->scheduleList->clear();
    this->com->schedule.clear();
}

void CommandCenterForm::on_startButton_clicked(){
    qDebug("startButtonClicked");
    if(!this->com->schedule.isEmpty()){
        qDebug("emit startCommandCenter");
        emit startCommandCenter();
        // emit startCommandCenter(schedule);
    } else {
        ui->errorOutput->setText("No existing schedule!");
        ui->activeOutput->clear();
    }
}

void CommandCenterForm::on_stopButton_clicked(){
    qDebug("stopButtonClicked");
    emit stopCommandCenter();
    ui->activeOutput->clear();
    ui->errorOutput->setText("Command center stopped!");
}

void CommandCenterForm::updateCcUI(QString s){
    ui->activeOutput->setText(s);
    ui->errorOutput->clear();
}
