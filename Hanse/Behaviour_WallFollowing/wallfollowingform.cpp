#include "wallfollowingform.h"
#include "ui_wallfollowingform.h"
#include <QFileDialog>

WallFollowingForm::WallFollowingForm(QWidget *parent, Behaviour_WallFollowing *wallfollowing) :
    QWidget(parent),
    ui(new Ui::WallFollowingForm)
{
    ui->setupUi(this);
    QObject::connect(this,SIGNAL(startBehaviour()),wallfollowing,SLOT(start()));
    QObject::connect(this,SIGNAL(stopBehaviour()),wallfollowing,SLOT(stop()));

 }

WallFollowingForm::~WallFollowingForm()
{
    delete ui;
}

void WallFollowingForm::changeEvent(QEvent *e)
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

void WallFollowingForm::on_startButton_clicked()
{
    emit startBehaviour();
}

void WallFollowingForm::on_stopButton_clicked()
{
    emit stopBehaviour();
}
