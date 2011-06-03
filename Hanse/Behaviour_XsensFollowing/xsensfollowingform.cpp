#include "xsensfollowingform.h"
#include "ui_xsensfollowingform.h"
#include <QFileDialog>

XsensFollowingForm::XsensFollowingForm(QWidget *parent, Behaviour_XsensFollowing *xsens) :
    QWidget(parent),
    ui(new Ui::XsensFollowingForm)
{
    ui->setupUi(this);
    this->xsens = xsens;
    this->ui->turnClockwise->setChecked(xsens->getSettingsValue("turnClockwise").toBool());
    this->ui->ffSpeed->setText(xsens->getSettingsValue("ffSpeed").toString());
    this->ui->driveTime->setText(xsens->getSettingsValue("driveTime").toString());
    this->ui->kp->setText(xsens->getSettingsValue("kp").toString());
    this->ui->delta->setText(xsens->getSettingsValue("delta").toString());
    this->ui->timerInput->setText(xsens->getSettingsValue("timer").toString());

    QObject::connect(this,SIGNAL(startBehaviour()),xsens,SLOT(startBehaviour()));
    QObject::connect(this,SIGNAL(stopBehaviour()),xsens,SLOT(stop()));

    QObject::connect(this,SIGNAL(refreshHeading()),xsens,SLOT(refreshHeading()));

 }

XsensFollowingForm::~XsensFollowingForm()
{
    delete ui;
}

void XsensFollowingForm::changeEvent(QEvent *e)
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

void XsensFollowingForm::on_startButton_clicked()
{
    xsens->setSettingsValue("driveTime",this->ui->driveTime->text().toInt());
    xsens->setSettingsValue("ffSpeed",ui->ffSpeed->text().toFloat());
    xsens->setSettingsValue("turnClockwise",this->ui->turnClockwise->isChecked());
    xsens->setSettingsValue("kp",ui->kp->text().toFloat());
    xsens->setSettingsValue("delta",ui->delta->text().toFloat());
    xsens->setSettingsValue("timer", ui->timerInput->text().toFloat());
    xsens->setSettingsValue("enableTurn",this->ui->enableTurn->isChecked());
    emit startBehaviour();
}

void XsensFollowingForm::on_stopButton_clicked()
{
    emit stopBehaviour();
}

void XsensFollowingForm::on_apply_clicked()
{
    xsens->setSettingsValue("driveTime",this->ui->driveTime->text().toInt());
    xsens->setSettingsValue("ffSpeed",ui->ffSpeed->text().toFloat());
    xsens->setSettingsValue("turnClockwise",this->ui->turnClockwise->isChecked());
    xsens->setSettingsValue("kp",ui->kp->text().toFloat());
    xsens->setSettingsValue("delta",ui->delta->text().toFloat());
    xsens->setSettingsValue("timer", ui->timerInput->text().toFloat());
    xsens->setSettingsValue("enableTurn",this->ui->enableTurn->isChecked());

}

void XsensFollowingForm::on_setHeading_clicked()
{
    emit refreshHeading();
}
