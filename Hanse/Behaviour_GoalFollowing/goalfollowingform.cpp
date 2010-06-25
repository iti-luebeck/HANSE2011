#include "goalfollowingform.h"
#include "ui_goalfollowingform.h"
#include <QFileDialog>

GoalFollowingForm::GoalFollowingForm(QWidget *parent, Behaviour_GoalFollowing *goalfollowing) :
    QWidget(parent),
    ui(new Ui::GoalFollowingForm)
{
    goalfollow = goalfollowing;
    ui->setupUi(this);

    ui->kpGoallineEdit->setText(goalfollow->getSettings().value("kpGoal").toString());
    ui->deltaGoalLineEdit->setText(goalfollow->getSettings().value("deltaGoal").toString());
    ui->robCenterXLineEdit->setText(goalfollow->getSettings().value("robCenterX").toString());
    ui->robCenterYLineEdit->setText(goalfollow->getSettings().value("robCenterY").toString());
    ui->fwSpeedLineEdit->setText(goalfollow->getSettings().value("fwSpeed").toString());
    ui->maxDistanceLineEdit->setText(goalfollow->getSettings().value("maxDistance").toString());
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
    goalfollow->start();;
}

void GoalFollowingForm::on_stopGoalFollowingButton_clicked()
{
    goalfollow->stop();

}

void GoalFollowingForm::on_saveAndApplyButton_clicked()
{
    goalfollow->getSettings().setValue("kpGoal",ui->kpGoallineEdit->text().toFloat());
    goalfollow->getSettings().setValue("deltaGoal",ui->deltaGoalLineEdit->text().toFloat());
    goalfollow->getSettings().setValue("robCenterX",ui->robCenterXLineEdit->text().toFloat());
    goalfollow->getSettings().setValue("robCenterY",ui->robCenterYLineEdit->text().toFloat());
    goalfollow->getSettings().setValue("fwSpeed",ui->fwSpeedLineEdit->text().toFloat());
    goalfollow->getSettings().setValue("maxDistance",ui->maxDistanceLineEdit->text().toFloat());
}
