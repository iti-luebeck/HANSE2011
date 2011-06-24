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

//    QObject::connect(this,SIGNAL(newSettingsPipeFollow(QString,QVariant)),pipefollow,SLOT(setSettingsValue(QString,QVariant)),Qt::BlockingQueuedConnection);
    QObject::connect(this,SIGNAL(settingsChanged()),pipefollow,SLOT(updateFromSettings()));

    QObject::connect(pipefollow,SIGNAL(dataChanged(RobotModule*)),this,SLOT(updatePixmap(RobotModule*)));

//    pipefollow->getSettings().setValue("videoFilePath",this->videoFile);
//    pipefollow->setSettingsValue("videoFilePath",this->videoFile);
//    emit newSettingsPipeFollow("videoFilePath",this->videoFile);

//    QObject::connect(this,SIGNAL(getSettingsValue(QString,QVariant&)),pipefollow,SLOT(getSettingsValueSl(QString,QVariant&)),Qt::DirectConnection);


    ui->thresholdLineEdit->setText(pipefollow->getSettingsValue("threshold").toString());
    ui->timer_LineEdit->setText(pipefollow->getSettingsValue("timer").toString());
    ui->deltaDistPipeLineEdit->setText(pipefollow->getSettingsValue("deltaDist").toString());
    ui->deltaAnglePipeLineEdit->setText(pipefollow->getSettingsValue("deltaAngle").toString());
    ui->kpDistLineEdit->setText(pipefollow->getSettingsValue("kpDist").toString());
    ui->kpAngleLineEdit->setText(pipefollow->getSettingsValue("kpAngle").toString());
    ui->robCenterXLineEdit->setText(pipefollow->getSettingsValue("robCenterX").toString());
    ui->robCenterYLineEdit->setText(pipefollow->getSettingsValue("robCenterY").toString());


    ui->maxDistLineEdti->setText(pipefollow->getSettingsValue("maxDistance").toString());
    ui->speedFwLineEdit->setText(pipefollow->getSettingsValue("fwSpeed").toString());
    ui->badFramesLineEdit->setText(pipefollow->getSettingsValue("badFrames").toString());
    ui->hRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 1);
    ui->sRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 2);
    ui->vRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 3);
    ui->grayRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 0);
    ui->hsvRadioButton->setChecked(pipefollow->getSettingsValue("convColor").toInt() == 4);
    ui->checkBox->setChecked(pipefollow->getSettingsValue("enableUIOutput").toBool());
    ui->frameColorCB->setChecked(pipefollow->getSettingsValue("frameOutput").toBool());
    QObject::connect(this, SIGNAL(startPipeFollow()),pipefollow , SLOT(startBehaviour()));
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
//    QTimer::singleShot(0, &updateUI,SLOT(start()));

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

void PipeFollowingForm::on_saveApplyButton_clicked()
{
    pipefollow->setSettingsValue("threshold",ui->thresholdLineEdit->text().toInt());
    pipefollow->setSettingsValue("timer",ui->timer_LineEdit->text().toInt());
    pipefollow->setSettingsValue("deltaDist",ui->deltaDistPipeLineEdit->text().toFloat());
    pipefollow->setSettingsValue("deltaAngle",ui->deltaAnglePipeLineEdit->text().toFloat());
    pipefollow->setSettingsValue("kpDist",ui->kpDistLineEdit->text().toFloat());
    pipefollow->setSettingsValue("kpAngle",ui->kpAngleLineEdit->text().toFloat());
    pipefollow->setSettingsValue("robCenterX",ui->robCenterXLineEdit->text().toDouble());
    pipefollow->setSettingsValue("robCenterY",ui->robCenterYLineEdit->text().toDouble());
    pipefollow->setSettingsValue("maxDistance",ui->maxDistLineEdti->text().toFloat());
    pipefollow->setSettingsValue("fwSpeed",ui->speedFwLineEdit->text().toFloat());
    pipefollow->setSettingsValue("badFrames",ui->badFramesLineEdit->text().toInt());
    pipefollow->setSettingsValue("frameOutput",ui->frameColorCB->isChecked());
    if(ui->hRadioButton->isChecked())
        pipefollow->setSettingsValue("convColor",1);
    else if(ui->sRadioButton->isChecked())
        pipefollow->setSettingsValue("convColor",2);
    else if(ui->vRadioButton->isChecked())
        pipefollow->setSettingsValue("convColor",3);
    else if(ui->grayRadioButton->isChecked())
        pipefollow->setSettingsValue("convColor",0);
    else if(ui->hsvRadioButton->isChecked())
        pipefollow->setSettingsValue("convColor",0);

    emit settingsChanged();
}

void PipeFollowingForm::updatePixmap(RobotModule*)
{
    if (ui->checkBox->isChecked()) {
        cv::Mat frame;
        pipefollow->grabFrame(frame);
        if(!frame.empty()) {
            QImage image1((unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
            ui->curPipeFrameLabel->setPixmap(QPixmap::fromImage(image1.rgbSwapped()));
        } else {
            ui->curPipeFrameLabel->setText("Empty Frame");
        }
    }
}

void PipeFollowingForm::on_stopButton_clicked()
{
    emit stopPipeFollow();
}

void PipeFollowingForm::on_checkBox_clicked()
{
    if(ui->checkBox->isChecked()) {
        pipefollow->setSettingsValue("enableUIOutput", true);
    } else if (!ui->checkBox->isChecked()) {
        pipefollow->setSettingsValue("enableUIOutput", false);
    }
}

void PipeFollowingForm::on_frameColorCB_clicked()
{
    pipefollow->setSettingsValue("frameOutput",ui->frameColorCB->isChecked());
}

void PipeFollowingForm::on_testButton_clicked()
{
    videoFile = QFileDialog::getExistingDirectory( this, "Open dir", "" );
    pipefollow->setSettingsValue("video directory", videoFile);
    QTimer::singleShot(0, pipefollow, SLOT(analyzeVideo()));
//    pipefollow->analyzeVideo(videoFile);
}
