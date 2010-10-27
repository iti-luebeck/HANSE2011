#include "pipefollowingform.h"
#include "ui_pipefollowingform.h"
#include <QFileDialog>

PipeFollowingForm::PipeFollowingForm(QWidget *parent, Behaviour_PipeFollowing *pipefollowing) :
    QWidget(parent),
    ui(new Ui::PipeFollowingForm)
{
    pipefollow = pipefollowing;
    ui->setupUi(this);

    qRegisterMetaType<QVariant>("QVariant");


    QObject::connect(this,SIGNAL(newDataPipeFollow(QString,QVariant)),pipefollow,SLOT(addData(QString,QVariant)));
    QObject::connect(this,SIGNAL(newSettingsPipeFollow(QString,QVariant)),pipefollow,SLOT(setSettingsValue(QString,QVariant)),Qt::BlockingQueuedConnection);
    QObject::connect(this,SIGNAL(settingsChanged()),pipefollow,SLOT(updateFromSettings()));

    this->videoFile = "../../../pipe_handy.avi" ;

    QObject::connect(&updateUI,SIGNAL(timeout()),this,SLOT(updatePixmap()));
//    QTimer::singleShot(10, &updateUI,SLOT(start()));

//    pipefollow->getSettings().setValue("videoFilePath",this->videoFile);
//    pipefollow->setSettingsValue("videoFilePath",this->videoFile);
//    emit newSettingsPipeFollow("videoFilePath",this->videoFile);

//    QObject::connect(this,SIGNAL(getSettingsValue(QString,QVariant&)),pipefollow,SLOT(getSettingsValueSl(QString,QVariant&)),Qt::DirectConnection);

//    QVariant text;
//    emit getSettingsValue("videoFilePath",text);
//    ui->curVideofileLabel->setText(text.toString());
//    emit getSettingsValue("threshold",text);
//    ui->thresholdLineEdit->setText(text.toString());
//    emit getSettingsValue("timer",text);
//    ui->timer_LineEdit->setText(text.toString());
//    emit getSettingsValue("deltaDist",text);
//    ui->deltaDistPipeLineEdit->setText(text.toString());
//    emit getSettingsValue("deltaAngle",text);
//    ui->deltaAnglePipeLineEdit->setText(text.toString());
//    emit getSettingsValue("kpDist",text);
//    ui->kpDistLineEdit->setText(text.toString());
//    emit getSettingsValue("kpAngle",text);
//    ui->kpAngleLineEdit->setText(text.toString());
//    emit getSettingsValue("robCenterX",text);
//    ui->robCenterXLineEdit->setText(text.toString());
//    emit getSettingsValue("robCenterY",text);
//    ui->robCenterYLineEdit->setText(text.toString());
//    emit getSettingsValue("debug",text);
//    ui->debugCheckBox->setChecked(text.toBool());
//    emit getSettingsValue("maxDistance",text);
//    ui->maxDistLineEdti->setText(text.toString());
//    emit getSettingsValue("fwSpeed",text);
//    ui->speedFwLineEdit->setText(text.toString());
//    emit getSettingsValue("camHeight",text);
//    ui->camHeightLineEdit->setText(text.toString());
//    emit getSettingsValue("camWidth",text);
//    ui->camWidthLineEdit->setText(text.toString());
//    emit getSettingsValue("badFrames",text);
//    ui->badFramesLineEdit->setText(text.toString());
//    emit getSettingsValue("convColor",text);
//    ui->hRadioButton->setChecked(text.toInt() == 1);
//    ui->sRadioButton->setChecked(text.toInt() == 2);
//    ui->vRadioButton->setChecked(text.toInt() == 3);
//    ui->grayRadioButton->setChecked(text.toInt() == 4);
//    ui->hsvRadioButton->setChecked(text.toInt() == 0);


    ui->curVideofileLabel->setText(pipefollow->getSettingsValue("videoFilePath").toString());
    ui->thresholdLineEdit->setText(pipefollow->getSettingsValue("threshold").toString());
    ui->timer_LineEdit->setText(pipefollow->getSettingsValue("timer").toString());
    ui->deltaDistPipeLineEdit->setText(pipefollow->getSettingsValue("deltaDist").toString());
    ui->deltaAnglePipeLineEdit->setText(pipefollow->getSettingsValue("deltaAngle").toString());
    ui->kpDistLineEdit->setText(pipefollow->getSettingsValue("kpDist").toString());
    ui->kpAngleLineEdit->setText(pipefollow->getSettingsValue("kpAngle").toString());
    ui->robCenterXLineEdit->setText(pipefollow->getSettingsValue("robCenterX").toString());
    ui->robCenterYLineEdit->setText(pipefollow->getSettingsValue("robCenterY").toString());
    ui->debugCheckBox->setChecked(pipefollow->getSettingsValue("debug").toBool());
    ui->useCameraRadioButton->setChecked(pipefollow->getSettingsValue("useCamera").toBool());
    ui->maxDistLineEdti->setText(pipefollow->getSettingsValue("maxDistance").toString());
    ui->speedFwLineEdit->setText(pipefollow->getSettingsValue("fwSpeed").toString());
    ui->camHeightLineEdit->setText(pipefollow->getSettingsValue("camHeight").toString());
    ui->camWidthLineEdit->setText(pipefollow->getSettingsValue("camWidth").toString());
    ui->badFramesLineEdit->setText(pipefollow->getSettingsValue("badFrames").toString());
    ui->hRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 1);
    ui->sRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 2);
    ui->vRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 3);
    ui->grayRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 4);
    ui->hsvRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 0);

//    ui->curVideofileLabel->setText(pipefollow->getSettings().value("videoFilePath").toString());
//    ui->thresholdLineEdit->setText(pipefollow->getSettings().value("threshold").toString());
//    ui->timer_LineEdit->setText(pipefollow->getSettings().value("timer").toString());
//    ui->deltaDistPipeLineEdit->setText(pipefollow->getSettings().value("deltaDist").toString());
//    ui->deltaAnglePipeLineEdit->setText(pipefollow->getSettings().value("deltaAngle").toString());
//    ui->kpDistLineEdit->setText(pipefollow->getSettings().value("kpDist").toString());
//    ui->kpAngleLineEdit->setText(pipefollow->getSettings().value("kpAngle").toString());
//    ui->robCenterXLineEdit->setText(pipefollow->getSettings().value("robCenterX").toString());
//    ui->robCenterYLineEdit->setText(pipefollow->getSettings().value("robCenterY").toString());
//    ui->debugCheckBox->setChecked(pipefollow->getSettings().value("debug").toBool());
//    ui->useCameraRadioButton->setChecked(pipefollow->getSettings().value("useCamera").toBool());
//    ui->maxDistLineEdti->setText(pipefollow->getSettings().value("maxDistance").toString());
//    ui->speedFwLineEdit->setText(pipefollow->getSettings().value("fwSpeed").toString());
//    ui->camHeightLineEdit->setText(pipefollow->getSettings().value("camHeight").toString());
//    ui->camWidthLineEdit->setText(pipefollow->getSettings().value("camWidth").toString());
//    ui->badFramesLineEdit->setText(pipefollow->getSettings().value("badFrames").toString());
//    ui->hRadioButton->setChecked(pipefollow->getSettings().value("convColor").toInt() == 1);
//    ui->sRadioButton->setChecked(pipefollow->getSettings().value("convColor").toInt() == 2);
//    ui->vRadioButton->setChecked(pipefollow->getSettings().value("convColor").toInt() == 3);
//    ui->grayRadioButton->setChecked(pipefollow->getSettings().value("convColor").toInt() == 4);
//    ui->hsvRadioButton->setChecked(pipefollow->getSettings().value("convColor").toInt() == 0);

    QObject::connect(this, SIGNAL(startPipeFollow()),pipefollow , SLOT(start()));
    QObject::connect(this, SIGNAL(stopPipeFollow()), pipefollow , SLOT(stop()));
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
//    pipefollow->getSettings().setValue("useCamera", true /*ui->useCameraRadioButton->isChecked()*/);
    emit startPipeFollow();
//    pipefollow->start();
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
//    videoFile = QFileDialog::getExistingDirectory( this, "Open dir", "" );
//    pipefollow->getSettings().setValue( "videoFilePath", videoFile );
//    ui->curVideofileLabel->setText( videoFile );
//
////    pipefollow->setDebug(ui->debugCheckBox->isChecked());
////    pipefollow->setThresh(ui->thresholdLineEdit->text().toInt());
//    pipefollow->analyzeVideo(videoFile);
}

void PipeFollowingForm::on_saveApplyButton_clicked()
{
    qDebug() << "pipeform thread id";
    qDebug() << QThread::currentThreadId();

    emit newSettingsPipeFollow("useCamera",ui->useCameraRadioButton->isChecked());
    emit newSettingsPipeFollow("threshold",ui->thresholdLineEdit->text().toInt());
    emit newSettingsPipeFollow("timer",ui->timer_LineEdit->text().toInt());
    emit newSettingsPipeFollow("deltaDist",ui->deltaDistPipeLineEdit->text().toFloat());
    emit newSettingsPipeFollow("deltaAngle",ui->deltaAnglePipeLineEdit->text().toFloat());
    emit newSettingsPipeFollow("kpDist",ui->kpDistLineEdit->text().toFloat());
    emit newSettingsPipeFollow("kpAngle",ui->kpAngleLineEdit->text().toFloat());
    emit newSettingsPipeFollow("robCenterX",ui->robCenterXLineEdit->text().toDouble());
    emit newSettingsPipeFollow("robCenterY",ui->robCenterYLineEdit->text().toDouble());
    emit newSettingsPipeFollow("debug",ui->debugCheckBox->isChecked());
    emit newSettingsPipeFollow("videoFilePath",ui->curVideofileLabel->text());
    emit newSettingsPipeFollow("maxDistance",ui->maxDistLineEdti->text().toFloat());
    emit newSettingsPipeFollow("fwSpeed",ui->speedFwLineEdit->text().toFloat());
    emit newSettingsPipeFollow("camHeight",ui->camHeightLineEdit->text().toInt());
    emit newSettingsPipeFollow("camWidth",ui->camWidthLineEdit->text().toInt());
    emit newSettingsPipeFollow("badFrames",ui->badFramesLineEdit->text().toInt());
    emit newSettingsPipeFollow("channel", ui->channelEdit->text().toInt());

    if(ui->hRadioButton->isChecked())
        emit newSettingsPipeFollow("convColor",1);
    else if(ui->sRadioButton->isChecked())
        emit newSettingsPipeFollow("convColor",2);
    else if(ui->vRadioButton->isChecked())
        emit newSettingsPipeFollow("convColor",3);
    else if(ui->grayRadioButton->isChecked())
        emit newSettingsPipeFollow("convColor",4);
    else if(ui->hsvRadioButton->isChecked())
        emit newSettingsPipeFollow("convColor",0);

    emit settingsChanged();
//    pipefollow->getSettings().setValue("useCamera",ui->useCameraRadioButton->isChecked());
//    pipefollow->getSettings().setValue("threshold",ui->thresholdLineEdit->text().toInt());
//    pipefollow->getSettings().setValue("timer",ui->timer_LineEdit->text().toInt());
//    pipefollow->getSettings().setValue("deltaDist",ui->deltaDistPipeLineEdit->text().toFloat());
//    pipefollow->getSettings().setValue("deltaAngle",ui->deltaAnglePipeLineEdit->text().toFloat());
//    pipefollow->getSettings().setValue("kpDist",ui->kpDistLineEdit->text().toFloat());
//    pipefollow->getSettings().setValue("kpAngle",ui->kpAngleLineEdit->text().toFloat());
//    pipefollow->getSettings().setValue("robCenterX",ui->robCenterXLineEdit->text().toDouble());
//    pipefollow->getSettings().setValue("robCenterY",ui->robCenterYLineEdit->text().toDouble());
//    pipefollow->getSettings().setValue("debug",ui->debugCheckBox->isChecked());
//    pipefollow->getSettings().setValue("videoFilePath",ui->curVideofileLabel->text());
//    pipefollow->getSettings().setValue("maxDistance",ui->maxDistLineEdti->text().toFloat());
//    pipefollow->getSettings().setValue("fwSpeed",ui->speedFwLineEdit->text().toFloat());
//    pipefollow->getSettings().setValue("camHeight",ui->camHeightLineEdit->text().toInt());
//    pipefollow->getSettings().setValue("camWidth",ui->camWidthLineEdit->text().toInt());
//    pipefollow->getSettings().setValue("badFrames",ui->badFramesLineEdit->text().toInt());
//    pipefollow->getSettings().setValue("channel", ui->channelEdit->text().toInt());
//    if(ui->hRadioButton->isChecked())
//        pipefollow->getSettings().setValue("convColor",1);
//    else if(ui->sRadioButton->isChecked())
//        pipefollow->getSettings().setValue("convColor",2);
//    else if(ui->vRadioButton->isChecked())
//        pipefollow->getSettings().setValue("convColor",3);
//    else if(ui->grayRadioButton->isChecked())
//        pipefollow->getSettings().setValue("convColor",4);
//    else if(ui->hsvRadioButton->isChecked())
//        pipefollow->getSettings().setValue("convColor",0);
//    pipefollow->updateFromSettings();
}

void PipeFollowingForm::updatePixmap()
{
    cv::Mat frame;
    pipefollow->grabFrame(frame);
    if(!frame.empty())
    {
   QImage image1((unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
//    QImage image1;
    ui->curPipeFrameLabel->setPixmap(QPixmap::fromImage(image1));

    }
     else
    {
        ui->curVideofileLabel->setText("Empty Frame");
    }

}

void PipeFollowingForm::on_stopButton_clicked()
{
    emit stopPipeFollow();
//    pipefollow->stop();
}


