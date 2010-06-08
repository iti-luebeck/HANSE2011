#include "pipefollowingform.h"
#include "ui_pipefollowingform.h"
#include <QFileDialog>

PipeFollowingForm::PipeFollowingForm(QWidget *parent, Behaviour_PipeFollowing *pipefollowing) :
    QWidget(parent),
    ui(new Ui::PipeFollowingForm)
{
    pipefollow = pipefollowing;
    ui->setupUi(this);

    videoFile = "../../../pipe_handy.avi";
    ui->curVideofileLabel->setText(videoFile);

 }

PipeFollowingForm::~PipeFollowingForm()
{
    delete ui;
}

void PipeFollowingForm::changeEvent(QEvent *e)
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

void PipeFollowingForm::on_startPipeFollowingButton_clicked()
{
    if(ui->useCameraRadioButton->isChecked())
    {
        pipefollow->start();
    }
    else if(ui->useVideofileRadioButton->isChecked())
    {
        pipefollow->setDebug(ui->debugCheckBox->isChecked());
        pipefollow->setThresh(ui->thresholdLineEdit->text().toInt());
        pipefollow->analyzeVideo(videoFile);
    }
}

void PipeFollowingForm::on_startFromVideoFileButton_clicked()
{
    pipefollow->setDebug(ui->debugCheckBox->isChecked());
    pipefollow->setThresh(ui->thresholdLineEdit->text().toInt());
    pipefollow->analyzeVideo(videoFile);
}

void PipeFollowingForm::on_openVideofileButton_clicked()
{
    videoFile = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mpg *.divx *.png)"));
    ui->curVideofileLabel->setText(videoFile);
}

void PipeFollowingForm::on_saveApplyButton_clicked()
{
    pipefollow->setThresh(ui->thresholdLineEdit->text().toInt());
    pipefollow->setCameraID(ui->cameraID_LineEdit->text().toInt());
    pipefollow->setDeltaPipe(ui->deltaPipeLineEdit->text().toDouble());
    pipefollow->setSpeed(ui->speedLineEdit->text().toDouble());
    pipefollow->setKp(ui->kpLineEdit->text().toDouble());
    pipefollow->setRobCenter(ui->robCenterXLineEdit->text().toDouble(),
                             ui->robCenterYLineEdit->text().toDouble());
    pipefollow->setDebug(ui->debugCheckBox->isChecked());


}
