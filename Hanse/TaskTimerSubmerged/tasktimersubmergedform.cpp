#include "tasktimersubmergedform.h"
#include "ui_tasktimersubmergedform.h"

TaskTimerSubmergedForm::TaskTimerSubmergedForm(TaskTimerSubmerged *tts, QWidget *parent) :
        QWidget(parent),
        ui(new Ui::TaskTimerSubmergedForm)
{
    ui->setupUi(this);
    this->tasktimersubmerged = tts;

    connect(tasktimersubmerged,SIGNAL(getUiSettings()),this,SLOT(on_applyButton_clicked()));
    this->ui->timerInput->setText(this->tasktimersubmerged->getSettingsValue("timerTime").toString());
    this->ui->depthInput->setText(this->tasktimersubmerged->getSettingsValue("targetDepth").toString());
}

TaskTimerSubmergedForm::~TaskTimerSubmergedForm()
{
    delete ui;
}

void TaskTimerSubmergedForm::changeEvent(QEvent *e)
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

void TaskTimerSubmergedForm::on_applyButton_clicked(){
    this->tasktimersubmerged->setSettingsValue("timerTime", this->ui->timerInput->text().toInt());
    this->tasktimersubmerged->setSettingsValue("targetDepth", this->ui->depthInput->text().toFloat());

}


