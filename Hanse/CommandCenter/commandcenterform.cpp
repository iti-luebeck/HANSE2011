#include "commandcenterform.h"
#include "ui_commandcenterform.h"

CommandCenterForm::CommandCenterForm(CommandCenter *commandcenter, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::CommandCenterForm)
{

    this->com = commandcenter;

    connect(this,SIGNAL(startCommandCenter()),com,SLOT(startCommandCenter()));
    connect(this,SIGNAL(stopCommandCenter()),com,SLOT(stopCommandCenter()));
    connect(this, SIGNAL(timerStartCommandCenter()), com, SLOT(timerStartCommandCenter()));

    // GUI-updating slots
    connect(com,SIGNAL(newMessage(QString)),this,SLOT(updateMessage(QString)));
    connect(com,SIGNAL(newError(QString)),this,SLOT(updateError(QString)));

    connect(com, SIGNAL(updateGUI()), this, SLOT(updateGUI()));
    connect(this, SIGNAL(updateGUISignal()), this, SLOT(updateGUI()));

    connect(com,SIGNAL(newState(QString)),this,SLOT(updateState(QString)));
    connect(com,SIGNAL(newStateOverview(QString)),this,SLOT(updateStateOverview(QString)));

    connect(this, SIGNAL(addTask(QString,QString)), com, SLOT(addTask(QString,QString)));
    connect(this, SIGNAL(clearList(QString)), com, SLOT(clearList(QString)));
    connect(this, SIGNAL(removeTask()), com, SLOT(removeTask()));

    connect(this, SIGNAL(skipTask()), com, SLOT(skipTask()));

    connect(com, SIGNAL(eSB()), this, SLOT(enableStartButton()));

    ui->setupUi(this);
    ui->depthInput->setText(com->getSettingsValue("targetDepth").toString());
    ui->waitInput->setText(com->getSettingsValue("waitTime").toString());
    ui->subBox->setChecked(com->getSettingsValue("subEx").toBool());
    ui->stopInput->setText(com->getSettingsValue("stopTime").toString());

    ui->useStartTimer->setChecked(com->getSettingsValue("useStartTimer").toBool());

    ui->scheduleInput->clear();
    for(int i = 0; i < this->com->taskInputList.length(); i++){
        ui->scheduleInput->addItem(this->com->taskInputList.at(i),"");
    }
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
    emit addTask("scheduleList",ui->scheduleInput->currentText());
    ui->errorOutput->setText("Added one task");
}

void CommandCenterForm::on_skipButton_clicked(){
    // Skip next task from schedule list
    emit skipTask();
    ui->errorOutput->setText("Next task skipped");
}


void CommandCenterForm::on_revertButton_clicked(){
    // Delete last scheduled task
    if(!this->com->scheduleList.isEmpty()){
        emit removeTask();
    }
    ui->errorOutput->setText("Reverted last add");
}

void CommandCenterForm::on_clearButton_clicked(){
    // Clear all scheduled tasks, but finish current working task
    emit clearList("scheduleList");
    ui->errorOutput->setText("Schedule list cleared");
}

void CommandCenterForm::on_startButton_clicked(){
    // Commandcenter start
    if(com->activeTask == ""){
        ui->finishedList->clear();
        ui->abortedList->clear();
        qDebug("Commandcenter start");
        if(!this->com->scheduleList.isEmpty()){
            // Set a few important values
            com->setSettingsValue("targetDepth", ui->depthInput->text());
            com->setSettingsValue("subEx", ui->subBox->isChecked());
            com->setSettingsValue("waitTime", ui->waitInput->text());
            com->setSettingsValue("stopTime", ui->stopInput->text());
            com->setSettingsValue("useStartTimer", ui->useStartTimer->isChecked());
            if(ui->useStartTimer->isChecked()){
                emit timerStartCommandCenter();
            } else {
                emit startCommandCenter();
            }
            this->ui->startButton->setDisabled(true);
        } else {
            qDebug("No task...");
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

    ui->activeOutput->clear();
    ui->stateOutput->clear();
    ui->stateOverviewOutput->clear();

    QString temp = "";

    if(!this->com->scheduleList.isEmpty()){
        for(int i = 0; i< this->com->scheduleList.length(); i++){
            temp = this->com->scheduleList.at(i);
            emit addTask("abortedList", temp);
        }
    }
    this->ui->startButton->setDisabled(false);
    ui->errorOutput->setText("Commandcenter stopped!");
    emit stopCommandCenter();
    emit updateGUISignal();
}

void CommandCenterForm::updateError(QString s){
    // Update GUI with current error or message
    ui->errorOutput->setText(s);
    ui->activeOutput->clear();
    ui->stateOutput->clear();
    ui->stateOverviewOutput->clear();
}

void CommandCenterForm::updateMessage(QString s){
    // Update GUI with current message
    ui->errorOutput->setText(s);
}

void CommandCenterForm::updateGUI(){
    ui->scheduleList->clear();
    for(int i = 0; i<=this->com->scheduleList.length()-1; i++){
        ui->scheduleList->insertPlainText(this->com->scheduleList.at(i)+"\n");
    }

    ui->abortedList->clear();
    for(int i = 0; i<=this->com->abortedList.length()-1; i++){
        ui->abortedList->insertPlainText(this->com->abortedList.at(i)+"\n");
    }

    ui->finishedList->clear();
    for(int i = 0; i<=this->com->finishedList.length()-1; i++){
        ui->finishedList->insertPlainText(this->com->finishedList.at(i)+"\n");
    }

    ui->activeOutput->clear();
    ui->activeOutput->setText(this->com->activeTask);

}

void CommandCenterForm::updateState(QString s){
    ui->stateOutput->setText(s);
}

void CommandCenterForm::updateStateOverview(QString s){
    if(s!="CLEAR"){
        ui->stateOverviewOutput->insertPlainText(s+"\n");
    } else {
        ui->stateOverviewOutput->clear();
    }
}

void CommandCenterForm::enableStartButton(){
    this->ui->startButton->setDisabled(false);
}
