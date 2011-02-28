#include "groundfollowingform.h"
#include "ui_groundfollowingform.h"

GroundFollowingForm::GroundFollowingForm(QWidget *parent, Behaviour_GroundFollowing *groundfollowing) :
    QWidget(parent),
    ui(new Ui::GroundFollowingForm)
{
    groundfollow = groundfollowing;
    ui->setupUi(this);
}

GroundFollowingForm::~GroundFollowingForm()
{
    delete ui;
}
