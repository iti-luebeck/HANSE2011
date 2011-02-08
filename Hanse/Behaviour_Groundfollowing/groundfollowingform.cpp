#include "groundfollowingform.h"
#include "ui_groundfollowingform.h"

groundfollowingform::groundfollowingform(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::groundfollowingform)
{
    ui->setupUi(this);
}

groundfollowingform::~groundfollowingform()
{
    delete ui;
}
