#include "pingerfollowingform.h"
#include "ui_pingerfollowingform.h"
#include <Framework/dataloghelper.h>


PingerFollowingForm::PingerFollowingForm(QWidget *parent, Behaviour_PingerFollowing *pingerfollowing) :
        QWidget(parent),
        ui(new Ui::PingerFollowingForm)
{
    ui->setupUi(this);
    this->pingerfollow = pingerfollowing;

    QObject::connect(this,SIGNAL(startBehaviour()),pingerfollow,SLOT(startBehaviour()));
    QObject::connect(this,SIGNAL(stopBehaviour()),pingerfollow,SLOT(stop()));


}

PingerFollowingForm::~PingerFollowingForm()
{
    delete ui;
}

void PingerFollowingForm::changeEvent(QEvent *e)
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

void PingerFollowingForm::on_startButton_clicked()
{


    emit startBehaviour();

}

void PingerFollowingForm::on_applyButton_clicked(){



}

void PingerFollowingForm::on_stopButton_clicked()
{
    emit stopBehaviour();
}
