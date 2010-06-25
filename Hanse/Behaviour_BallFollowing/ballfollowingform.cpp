#include "ballfollowingform.h"
#include "ui_ballfollowingform.h"
#include <QFileDialog>

BallFollowingForm::BallFollowingForm(QWidget *parent, Behaviour_BallFollowing *ballfollowing) :
    QWidget(parent),
    ui(new Ui::BallFollowingForm)
{
    ballfollow = ballfollowing;
    ui->setupUi(this);

    ui->kpBalllineEdit->setText(ballfollow->getSettings().value("kpBall").toString());
    ui->deltaBallLineEdit->setText(ballfollow->getSettings().value("deltaBall").toString());
    ui->robCenterXLineEdit->setText(ballfollow->getSettings().value("robCenterX").toString());
    ui->robCenterYLineEdit->setText(ballfollow->getSettings().value("robCenterY").toString());
    ui->fwSpeedLineEdit->setText(ballfollow->getSettings().value("fwSpeed").toString());
    ui->maxDistanceLineEdit->setText(ballfollow->getSettings().value("maxDistance").toString());
 }

BallFollowingForm::~BallFollowingForm()
{
    delete ui;
}

void BallFollowingForm::changeEvent(QEvent *e)
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



void BallFollowingForm::on_startBallFollwoingButton_clicked()
{
    ballfollow->start();;
}

void BallFollowingForm::on_stopBallFollowingButton_clicked()
{
    ballfollow->stop();

}

void BallFollowingForm::on_saveAndApplyButton_clicked()
{
    ballfollow->getSettings().setValue("kpBall",ui->kpBalllineEdit->text().toFloat());
    ballfollow->getSettings().setValue("deltaBall",ui->deltaBallLineEdit->text().toFloat());
    ballfollow->getSettings().setValue("robCenterX",ui->robCenterXLineEdit->text().toFloat());
    ballfollow->getSettings().setValue("robCenterY",ui->robCenterYLineEdit->text().toFloat());
    ballfollow->getSettings().setValue("fwSpeed",ui->fwSpeedLineEdit->text().toFloat());
    ballfollow->getSettings().setValue("maxDistance",ui->maxDistanceLineEdit->text().toFloat());
}
