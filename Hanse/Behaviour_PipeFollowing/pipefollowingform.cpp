#include "pipefollowingform.h"
#include "ui_pipefollowingform.h"
#include <QFileDialog>
#include <Framework/objecttrackerwidget.h>

PipeFollowingForm::PipeFollowingForm(QWidget *parent, Behaviour_PipeFollowing *pipefollowing) :
    QWidget(parent),
    ui(new Ui::PipeFollowingForm)
{
    pipefollow = pipefollowing;
    ui->setupUi(this);

//    qRegisterMetaType<QVariant>("QVariant");

    QLayout *l = new QBoxLayout(QBoxLayout::LeftToRight, ui->trackerFrame);
    ObjectTrackerWidget *trackerWidget = new ObjectTrackerWidget(pipefollowing->getTracker(), this);
    l->addWidget(trackerWidget);

    QObject::connect(this,SIGNAL(settingsChanged()),pipefollow,SLOT(updateFromSettings()));

    ui->timer_LineEdit->setText(pipefollow->getSettingsValue("timer").toString());
    ui->deltaDistPipeLineEdit->setText(pipefollow->getSettingsValue("deltaDist").toString());
    ui->deltaAnglePipeLineEdit->setText(pipefollow->getSettingsValue("deltaAngle").toString());
    ui->kpDistLineEdit->setText(pipefollow->getSettingsValue("kpDist").toString());
    ui->kpAngleLineEdit->setText(pipefollow->getSettingsValue("kpAngle").toString());
    ui->robCenterXLineEdit->setText(pipefollow->getSettingsValue("robCenterX").toString());
    ui->robCenterYLineEdit->setText(pipefollow->getSettingsValue("robCenterY").toString());
    ui->maxDistLineEdti->setText(pipefollow->getSettingsValue("maxDistance").toString());
    ui->speedFwLineEdit->setText(pipefollow->getSettingsValue("fwSpeed").toString());

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
    emit startPipeFollow();
}

void PipeFollowingForm::on_stopButton_clicked()
{
    emit stopPipeFollow();
}

void PipeFollowingForm::on_testButton_clicked()
{
    videoFile = QFileDialog::getExistingDirectory( this, "Open dir", "" );
    pipefollow->setSettingsValue("video directory", videoFile);
    QTimer::singleShot(0, pipefollow, SLOT(analyzeVideo()));
}

void PipeFollowingForm::on_saveApplyButton_clicked()
{
    pipefollow->setSettingsValue("timer",ui->timer_LineEdit->text().toInt());
    pipefollow->setSettingsValue("deltaDist",ui->deltaDistPipeLineEdit->text().toFloat());
    pipefollow->setSettingsValue("deltaAngle",ui->deltaAnglePipeLineEdit->text().toFloat());
    pipefollow->setSettingsValue("kpDist",ui->kpDistLineEdit->text().toFloat());
    pipefollow->setSettingsValue("kpAngle",ui->kpAngleLineEdit->text().toFloat());
    pipefollow->setSettingsValue("robCenterX",ui->robCenterXLineEdit->text().toDouble());
    pipefollow->setSettingsValue("robCenterY",ui->robCenterYLineEdit->text().toDouble());
    pipefollow->setSettingsValue("maxDistance",ui->maxDistLineEdti->text().toFloat());
    pipefollow->setSettingsValue("fwSpeed",ui->speedFwLineEdit->text().toFloat());

    emit settingsChanged();
}
