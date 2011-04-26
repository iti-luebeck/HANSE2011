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


    connect(com,SIGNAL(currentTask(QString)),this,SLOT(updateTask(QString)));
    connect(com,SIGNAL(newError(QString)),this,SLOT(updateError(QString)));

    connect(com,SIGNAL(newList(QString)),this,SLOT(updateLists(QString)));
    connect(com,SIGNAL(newAborted(QString)),this,SLOT(updateAborted(QString)));

    connect(this,SIGNAL(stopCommandCenter()),com,SLOT(stopCC()));

    ui->setupUi(this);
    ui->depthInput->setText(com->getSettingsValue("targetDepth").toString());
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
    ui->errorOutput->setText("Schedule list cleared");
}

void CommandCenterForm::on_startButton_clicked(){
    ui->finishedList->clear();
    qDebug("startButtonClicked");
    if(!this->com->schedule.isEmpty()){
        qDebug("emit startCommandCenter");
        emit startCommandCenter();
        // emit startCommandCenter(schedule);
    } else {
        ui->errorOutput->setText("No existing schedule!");
        ui->activeOutput->clear();
    }
    com->setSettingsValue("targetDepth", ui->depthInput->text());
    com->setSettingsValue("subEx", ui->subBox->isChecked());
}

void CommandCenterForm::on_stopButton_clicked(){
    qDebug("stopButtonClicked");

    ui->activeOutput->clear();
    for(int i = 0; i<this->com->schedule.length(); i++){
        ui->abortedList->insertPlainText(this->com->schedule.at(i)+"\n");
    }
    emit stopCommandCenter();
    ui->scheduleList->clear();
    ui->errorOutput->setText("Command center stopped!");

}

void CommandCenterForm::updateTask(QString s){
    ui->activeOutput->setText(s);
    ui->errorOutput->clear();
}

void CommandCenterForm::updateError(QString s){
    ui->errorOutput->setText(s);
    ui->activeOutput->clear();
}

void CommandCenterForm::updateLists(QString s){
    ui->scheduleList->clear();
    for(int i = this->com->schedule.length()-1; i>=0; i--){
        ui->scheduleList->insertPlainText(this->com->schedule.at(i)+"\n");
    }
    if(s!=""){
       ui->finishedList->insertPlainText(s+"\n");
    }
}

void CommandCenterForm::updateAborted(QString s){
    ui->scheduleList->clear();
    for(int i = this->com->schedule.length()-1; i>=0; i--){
        ui->scheduleList->insertPlainText(this->com->schedule.at(i)+"\n");
    }
    ui->abortedList->insertPlainText(s+"\n");
}
