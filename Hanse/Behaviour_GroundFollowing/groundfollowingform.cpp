#include "groundfollowingform.h"
#include "ui_groundfollowingform.h"

GroundFollowingForm::GroundFollowingForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroundFollowingForm)
{
    ui->setupUi(this);
}

GroundFollowingForm::~GroundFollowingForm()
{
    delete ui;
}
