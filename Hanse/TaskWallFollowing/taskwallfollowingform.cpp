#include "taskwallfollowingform.h"
#include "ui_taskwallfollowingform.h"

TaskWallFollowingForm::TaskWallFollowingForm(TaskWallFollowing *tw, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskWallFollowingForm)
{
    ui->setupUi(this);
    this->taskwallfollowing = tw;
}

TaskWallFollowingForm::~TaskWallFollowingForm()
{
    delete ui;
}

void TaskWallFollowingForm::changeEvent(QEvent *e)
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

void TaskWallFollowingForm::on_startButton_clicked(){
    qDebug("startButtonClicked");
}

void TaskWallFollowingForm::on_stopButton_clicked(){
    qDebug("stopButtonClicked");
}
