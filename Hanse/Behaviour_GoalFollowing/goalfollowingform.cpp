#include "goalfollowingform.h"
#include "ui_goalfollowingform.h"
#include <QFileDialog>

GoalFollowingForm::GoalFollowingForm(QWidget *parent, Behaviour_GoalFollowing *goalfollowing) :
    QWidget(parent),
    ui(new Ui::GoalFollowingForm)
{
    goalfollow = goalfollowing;
    ui->setupUi(this);

    ui->kpGoallineEdit->setText(goalfollow->getSettingsValue("kpGoal").toString());
    ui->deltaGoalLineEdit->setText(goalfollow->getSettingsValue("deltaGoal").toString());
    ui->robCenterXLineEdit->setText(goalfollow->getSettingsValue("robCenterX").toString());
    ui->robCenterYLineEdit->setText(goalfollow->getSettingsValue("robCenterY").toString());
    ui->fwSpeedLineEdit->setText(goalfollow->getSettingsValue("fwSpeed").toString());
    ui->maxDistanceLineEdit->setText(goalfollow->getSettingsValue("maxDistance").toString());
 }

GoalFollowingForm::~GoalFollowingForm()
{
    delete ui;
}

void GoalFollowingForm::changeEvent(QEvent *e)
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



void GoalFollowingForm::on_startGoalFollwoingButton_clicked()
{
    goalfollow->startBehaviour();
}

void GoalFollowingForm::on_stopGoalFollowingButton_clicked()
{
    goalfollow->stop();

}

void GoalFollowingForm::on_saveAndApplyButton_clicked()
{
    goalfollow->setSettingsValue("kpGoal",ui->kpGoallineEdit->text().toFloat());
    goalfollow->setSettingsValue("deltaGoal",ui->deltaGoalLineEdit->text().toFloat());
    goalfollow->setSettingsValue("robCenterX",ui->robCenterXLineEdit->text().toFloat());
    goalfollow->setSettingsValue("robCenterY",ui->robCenterYLineEdit->text().toFloat());
    goalfollow->setSettingsValue("fwSpeed",ui->fwSpeedLineEdit->text().toFloat());
    goalfollow->setSettingsValue("maxDistance",ui->maxDistanceLineEdit->text().toFloat());
}
