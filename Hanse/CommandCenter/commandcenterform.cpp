#include "commandcenterform.h"
#include "ui_commandcenterform.h"

CommandCenterForm::CommandCenterForm(CommandCenter *commandcenter, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CommandCenterForm)
{

    this->com = commandcenter;

    // Very important connections with commandcenter
    connect(this,SIGNAL(startCommandCenter()),com,SLOT(startCommandCenter()));
    connect(this,SIGNAL(stopCommandCenter()),com,SLOT(stopCommandCenter()));
    connect(this,SIGNAL(setDescription()),com,SLOT(setDescriptionSlot()));

    // GUI-updating slots
    connect(com,SIGNAL(currentTask(QString)),this,SLOT(updateTask(QString)));
    connect(com,SIGNAL(newError(QString)),this,SLOT(updateError(QString)));
    connect(com,SIGNAL(newList(QString)),this,SLOT(updateLists(QString)));
    connect(com,SIGNAL(newAborted(QString)),this,SLOT(updateAborted(QString)));

    connect(com,SIGNAL(newMessage(QString)),this,SLOT(updateMessage(QString)));

    connect(com,SIGNAL(newSchDesSignal(QString,QString)),this,SLOT(refreshScheduleList(QString, QString)));


    ui->setupUi(this);
    ui->depthInput->setText(com->getSettingsValue("targetDepth").toString());
    ui->waitInput->setText(com->getSettingsValue("waitTime").toString());

    this->scheduledTasks.clear();
    this->scheduledTasks.append("Wall1");
    this->scheduledTasks.append("Wall2");
    this->scheduledTasks.append("Wall3");

    this->scheduledTasks.append("Pipe1");
    this->scheduledTasks.append("Pipe2");
    this->scheduledTasks.append("Pipe3");

    this->scheduledTasks.append("Turn1");
    this->scheduledTasks.append("Turn2");
    this->scheduledTasks.append("Turn3");

    this->scheduledTasks.append("Thruster1");
    this->scheduledTasks.append("Thruster2");
    this->scheduledTasks.append("Thruster3");
    this->scheduledTasks.append("Thruster4");
    this->scheduledTasks.append("Thruster5");
    this->scheduledTasks.append("Thruster6");
    this->scheduledTasks.append("HandControl");


    ui->scheduleInput->clear();
    for(int i = 0; i < this->scheduledTasks.length(); i++){
        ui->scheduleInput->addItem(this->scheduledTasks.at(i),"");
    }

    emit setDescription();
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
    // Add new task
    ui->scheduleList->insertPlainText(ui->scheduleInput->currentText()+"\n");
    this->com->schedule.prepend(ui->scheduleInput->currentText());
    //qDebug()<<ui->scheduleInput->currentText();
    ui->errorOutput->setText("Added one task");
}

void CommandCenterForm::on_revertButton_clicked(){
    // Delete last scheduled task
    if(!this->com->schedule.isEmpty()){
        this->com->schedule.removeFirst();
    }
    ui->scheduleList->clear();
    for(int i = this->com->schedule.length()-1; i>=0; i--){
        ui->scheduleList->insertPlainText(this->com->schedule.at(i)+"\n");
    }
    ui->errorOutput->setText("Reverted last add");
}

void CommandCenterForm::on_clearButton_clicked(){
    // Clear all scheduled tasks, but finish current working task
    ui->scheduleList->clear();
    this->com->schedule.clear();
    ui->errorOutput->setText("Schedule list cleared");
}

void CommandCenterForm::on_startButton_clicked(){
    // Commandcenter start
    if(!com->isEnabled()){
        com->count=1;
        ui->finishedList->clear();
        ui->abortedList->clear();
        if(!this->com->schedule.isEmpty()){
            qDebug("Commandcenter start");
            // Set a few important values
            com->setSettingsValue("targetDepth", ui->depthInput->text());
            com->setSettingsValue("subEx", ui->subBox->isChecked());
            com->setSettingsValue("waitTime", ui->waitInput->text());
            emit startCommandCenter();
        } else {
            ui->errorOutput->setText("No existing schedule!");
        }
    } else {
        ui->errorOutput->setText("Please stop the commandcenter first, schedule new tasks and start again!");
    }
}

void CommandCenterForm::on_stopButton_clicked(){
    qDebug("Commandcenter stop");
    // Clear the GUI and set aborted task
    ui->scheduleList->clear();
    emit stopCommandCenter();
    ui->activeOutput->clear();

    QString temp = "";

    for(int i = 0; i< this->com->schedule.length(); i++){
          temp = this->com->schedule.at(i);
          ui->abortedList->insertPlainText(temp+"\n");
    }

    ui->errorOutput->setText("Commandcenter stopped!");

}


void CommandCenterForm::updateTask(QString s){
    // Update GUI with current working task
    ui->activeOutput->setText(s);
    ui->errorOutput->clear();
}

void CommandCenterForm::updateError(QString s){
    // Update GUI with current error or message
    ui->errorOutput->setText(s);
    ui->activeOutput->clear();
}

void CommandCenterForm::updateMessage(QString s){
    // Update GUI with current message
    ui->errorOutput->setText(s);
}

void CommandCenterForm::updateLists(QString s){
    // Update GUI with current scheduled tasks and finished tasks
    ui->scheduleList->clear();
    for(int i = this->com->schedule.length()-1; i>=0; i--){
        ui->scheduleList->insertPlainText(this->com->schedule.at(i)+"\n");
    }
    if(s!=""){
        ui->finishedList->insertPlainText(s+"\n");
    }
}

void CommandCenterForm::updateAborted(QString s){
    // Update GUI with current aborted tasks and delete the aborted task from scheduled list
    ui->scheduleList->clear();
    for(int i = this->com->schedule.length()-1; i>=0; i--){
        ui->scheduleList->insertPlainText(this->com->schedule.at(i)+"\n");
    }
    ui->abortedList->insertPlainText(s+"\n");
}

void CommandCenterForm::refreshScheduleList(QString scheduleName, QString newD){
    QString temp1 = "";
    QString temp2 = "";
    for(int i= 0; i<scheduledTasks.length(); i++){
        temp1 = scheduledTasks.at(i);
        for(int j = 0; j < temp1.length(); j++){
            if(((temp1[j] == QChar(':'))==false) && ((temp1[j] == QChar(' '))==false) ){
                temp2 = temp2 + temp1.at(j);
            } else {
                break;
            }
        }
        if(temp2 == scheduleName){
            temp2 = temp2 + ": "+newD;
            scheduledTasks.replace(i, temp2);
            break;
        } else {
            temp2 = "";
        }
    }

    ui->scheduleInput->clear();
    for(int i = 0; i < this->scheduledTasks.length(); i++){
        ui->scheduleInput->addItem(this->scheduledTasks.at(i),"");
    }
}
