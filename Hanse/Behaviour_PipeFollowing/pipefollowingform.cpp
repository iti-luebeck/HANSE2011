#include "pipefollowingform.h"
#include "ui_pipefollowingform.h"
#include <QFileDialog>

PipeFollowingForm::PipeFollowingForm(QWidget *parent, Behaviour_PipeFollowing *pipefollowing) :
    QWidget(parent),
    ui(new Ui::PipeFollowingForm)
{
    pipefollow = pipefollowing;
    ui->setupUi(this);

    this->videoFile = "../../../pipe_handy.avi" ;
    pipefollow->getSettings().setValue("videoFilePath",this->videoFile);

    ui->curVideofileLabel->setText(pipefollow->getSettings().value("videFilePath").toString());
    ui->thresholdLineEdit->setText(pipefollow->getSettings().value("threshold").toString());
    ui->cameraID_LineEdit->setText(pipefollow->getSettings().value("cameraID").toString());
    ui->deltaDistPipeLineEdit->setText(pipefollow->getSettings().value("deltaDist").toString());
    ui->deltaAnglePipeLineEdit->setText(pipefollow->getSettings().value("deltaAngle").toString());
    ui->kpDistLineEdit->setText(pipefollow->getSettings().value("kpDist").toString());
    ui->kpAngleLineEdit->setText(pipefollow->getSettings().value("kpAngle").toString());
    ui->robCenterXLineEdit->setText(pipefollow->getSettings().value("robCenterX").toString());
    ui->robCenterYLineEdit->setText(pipefollow->getSettings().value("robCenterY").toString());
    ui->debugCheckBox->setChecked(pipefollow->getSettings().value("debug").toBool());
    ui->useCameraRadioButton->setChecked(pipefollow->getSettings().value("useCamera").toBool());
    ui->maxDistLineEdti->setText(pipefollow->getSettings().value("maxDistance").toString());
    ui->speedFwLineEdit->setText(pipefollow->getSettings().value("fwSpeed").toString());
    ui->camHeightLineEdit->setText(pipefollow->getSettings().value("camHeight").toString());
    ui->camWidthLineEdit->setText(pipefollow->getSettings().value("camWidth").toString());
    ui->badFramesLineEdit->setText(pipefollow->getSettings().value("badFrames").toString());

    QObject::connect( pipefollow, SIGNAL( printFrameOnUi(cv::Mat&)) , SLOT( printFrame(cv::Mat&))  );

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
    pipefollow->getSettings().setValue("useCamera",ui->useCameraRadioButton->isChecked());
    pipefollow->start();
//    if(ui->useCameraRadioButton->isChecked())
//    {
//        pipefollow->start();
//    }
//    else if(ui->useVideofileRadioButton->isChecked())
//    {
//        pipefollow->setDebug(ui->debugCheckBox->isChecked());
//        pipefollow->setThresh(ui->thresholdLineEdit->text().toInt());
//        pipefollow->analyzeVideo(videoFile);
//    }
}

void PipeFollowingForm::on_startFromVideoFileButton_clicked()
{
    pipefollow->setDebug(ui->debugCheckBox->isChecked());
    pipefollow->setThresh(ui->thresholdLineEdit->text().toInt());
    pipefollow->analyzeVideo(videoFile);
}

void PipeFollowingForm::on_openVideofileButton_clicked()
{
    QString file = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mpg *.divx *.png)"));
    pipefollow->getSettings().setValue("videoFilePath",file);
    ui->curVideofileLabel->setText(file);
}

void PipeFollowingForm::on_saveApplyButton_clicked()
{
    pipefollow->getSettings().setValue("useCamera",ui->useCameraRadioButton->isChecked());
    pipefollow->getSettings().setValue("threshold",ui->thresholdLineEdit->text().toInt());
    pipefollow->getSettings().setValue("cameraID",ui->cameraID_LineEdit->text().toInt());
    pipefollow->getSettings().setValue("deltaDist",ui->deltaDistPipeLineEdit->text().toFloat());
    pipefollow->getSettings().setValue("deltaAngle",ui->deltaAnglePipeLineEdit->text().toFloat());
    pipefollow->getSettings().setValue("kpDist",ui->kpDistLineEdit->text().toFloat());
    pipefollow->getSettings().setValue("kpAngle",ui->kpAngleLineEdit->text().toFloat());
    pipefollow->getSettings().setValue("robCenterX",ui->robCenterXLineEdit->text().toDouble());
    pipefollow->getSettings().setValue("robCenterY",ui->robCenterYLineEdit->text().toDouble());
    pipefollow->getSettings().setValue("debug",ui->debugCheckBox->isChecked());
    pipefollow->getSettings().setValue("videoFilePath",ui->curVideofileLabel->text());
    pipefollow->getSettings().setValue("maxDistance",ui->maxDistLineEdti->text().toFloat());
    pipefollow->getSettings().setValue("fwSpeed",ui->speedFwLineEdit->text().toFloat());
    pipefollow->getSettings().setValue("camHeight",ui->camHeightLineEdit->text().toInt());
    pipefollow->getSettings().setValue("camWidth",ui->camWidthLineEdit->text().toInt());
    pipefollow->getSettings().setValue("badFrames",ui->badFramesLineEdit->text().toInt());
    pipefollow->resetFirstRun();
    pipefollow->updateFromSettings();

}

void PipeFollowingForm::printFrame(cv::Mat &frame)
{

    QImage image1((unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_Indexed8);
//    QImage image1;
    ui->curPipeFrameLabel->setPixmap(QPixmap::fromImage(image1));
//    ui->curVideofileLabel->setText("Blub");

}

void PipeFollowingForm::on_stopButton_clicked()
{
    pipefollow->stop();
}
