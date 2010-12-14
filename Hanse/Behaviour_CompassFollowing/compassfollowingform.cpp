#include "compassfollowingform.h"
#include "ui_compassfollowingform.h"
#include <QFileDialog>

CompassFollowingForm::CompassFollowingForm(QWidget *parent, Behaviour_CompassFollowing *comp) :
    QWidget(parent),
    ui(new Ui::CompassFollowingForm)
{
    ui->setupUi(this);
    this->comp = comp;
    QObject::connect(this,SIGNAL(startBehaviour()),comp,SLOT(start()));
    QObject::connect(this,SIGNAL(stopBehaviour()),comp,SLOT(stop()));

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
