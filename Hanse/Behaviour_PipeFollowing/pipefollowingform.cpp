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
    pipefollow->setDeltaPipe(ui->deltaDistPipeLineEdit->text().toFloat() ,
                             ui->deltaAnglePipeLineEdit->text().toFloat());
    pipefollow->setKpDist(ui->kpDistLineEdit->text().toFloat());
    pipefollow->setKpAngle(ui->kpAngleLineEdit->text().toFloat());
    pipefollow->setRobCenter(ui->robCenterXLineEdit->text().toDouble(),
                             ui->robCenterYLineEdit->text().toDouble());
    pipefollow->setDebug(ui->debugCheckBox->isChecked());

}

void PipeFollowingForm::printFrame(cv::Mat &frame)
{

    QImage image1((unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
//    QImage image1;
    ui->curPipeFrameLabel->setPixmap(QPixmap::fromImage(image1));
//    ui->curVideofileLabel->setText("Blub");

}
