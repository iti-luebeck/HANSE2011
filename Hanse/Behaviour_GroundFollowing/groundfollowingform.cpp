#include "groundfollowingform.h"
#include "ui_groundfollowingform.h"
#include <QFileDialog>

GroundFollowingForm::GroundFollowingForm(QWidget *parent, Behaviour_GroundFollowing *groundfollowing) :
    QWidget(parent),
    ui(new Ui::GroundFollowingForm)
{
    groundfollow = groundfollowing;
    ui->setupUi(this);

    qRegisterMetaType<QVariant>("QVariant");

    QObject::connect(this,SIGNAL(settingsChanged()),groundfollow,SLOT(updateFromSettings()));
}

GroundFollowingForm::~GroundFollowingForm()
{
    delete ui;
}

void GroundFollowingForm::changeEvent(QEvent *e)
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

void GroundFollowingForm::on_startButton_clicked()
{


}


void GroundFollowingForm::on_saveButton_clicked()
{
    qDebug() << "groundform thread id";
    qDebug() << QThread::currentThreadId();

}

void GroundFollowingForm::on_stopButton_clicked()
{

}

