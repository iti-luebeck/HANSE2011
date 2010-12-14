#include "compassfollowingform.h"
#include "ui_compassfollowingform.h"
#include <QFileDialog>

CompassFollowingForm::CompassFollowingForm(QWidget *parent, Behaviour_CompassFollowing *comp) :
    QWidget(parent),
    ui(new Ui::CompassFollowingForm)
{
    ui->setupUi(this);
    this->comp = comp;
    this->ui->turnClockwise->setChecked(comp->getSettingsValue("turnClockwise").toBool());
    this->ui->ffSpeed->setText(comp->getSettingsValue("ffSpeed").toString());
    this->ui->driveTime->setText(comp->getSettingsValue("driveTime").toString());
    this->ui->kp->setText(comp->getSettingsValue("kp").toString());
    this->ui->delta->setText(comp->getSettingsValue("delta").toString());

    QObject::connect(this,SIGNAL(startBehaviour()),comp,SLOT(start()));
    QObject::connect(this,SIGNAL(stopBehaviour()),comp,SLOT(stop()));

    QObject::connect(this,SIGNAL(refresh()),comp,SLOT(refreshHeading()));

 }

CompassFollowingForm::~CompassFollowingForm()
{
    delete ui;
}

void CompassFollowingForm::changeEvent(QEvent *e)
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

void CompassFollowingForm::on_startButton_clicked()
{
    emit startBehaviour();
}

void CompassFollowingForm::on_stopButton_clicked()
{
    emit stopBehaviour();
}

void CompassFollowingForm::on_apply_clicked()
{
    comp->setSettingsValue("driveTime",this->ui->driveTime->text());
    comp->setSettingsValue("ffSpeed",ui->ffSpeed->text());
    comp->setSettingsValue("turnClockwise",this->ui->turnClockwise->isChecked());
    comp->setSettingsValue("kp",ui->kp->text());
    comp->setSettingsValue("delta",ui->delta->text());
}

void CompassFollowingForm::on_setHeading_clicked()
{
    emit refreshHeading();
}
