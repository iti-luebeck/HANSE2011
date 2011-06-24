#include "ballfollowingform.h"
#include "ui_ballfollowingform.h"
#include <QFileDialog>

BallFollowingForm::BallFollowingForm(QWidget *parent, Behaviour_BallFollowing *ballfollowing) :
    QWidget(parent),
    ui(new Ui::BallFollowingForm)
{
    ballfollow = ballfollowing;
    ui->setupUi(this);

    ui->kpBalllineEdit->setText(ballfollow->getSettingsValue("kpBall").toString());
    ui->deltaBallLineEdit->setText(ballfollow->getSettingsValue("deltaBall").toString());
    ui->robCenterXLineEdit->setText(ballfollow->getSettingsValue("robCenterX").toString());
    ui->robCenterYLineEdit->setText(ballfollow->getSettingsValue("robCenterY").toString());
    ui->fwSpeedLineEdit->setText(ballfollow->getSettingsValue("fwSpeed").toString());
    ui->maxDistanceLineEdit->setText(ballfollow->getSettingsValue("maxDistance").toString());
    ui->thresholdEdit->setText( ballfollow->getSettingsValue( "threshold", 100 ).toString() );

    QObject::connect( ballfollowing, SIGNAL(dataChanged(RobotModule*)),
                      this, SLOT(printFrame(RobotModule*)) );
    QObject::connect( this, SIGNAL(doTest(QString)),
                      ballfollowing, SLOT(testBehaviour(QString)) );
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
    ballfollow->startBehaviour();
}

void BallFollowingForm::on_stopBallFollowingButton_clicked()
{
    ballfollow->stop();

}

void BallFollowingForm::on_saveAndApplyButton_clicked()
{
    ballfollow->setSettingsValue("kpBall",ui->kpBalllineEdit->text().toFloat());
    ballfollow->setSettingsValue("deltaBall",ui->deltaBallLineEdit->text().toFloat());
    ballfollow->setSettingsValue("robCenterX",ui->robCenterXLineEdit->text().toFloat());
    ballfollow->setSettingsValue("robCenterY",ui->robCenterYLineEdit->text().toFloat());
    ballfollow->setSettingsValue("fwSpeed",ui->fwSpeedLineEdit->text().toFloat());
    ballfollow->setSettingsValue("maxDistance",ui->maxDistanceLineEdit->text().toFloat());
    ballfollow->setSettingsValue("threshold",ui->thresholdEdit->text().toInt());
}

void BallFollowingForm::printFrame(RobotModule*)
{
    cv::Mat frame;
    ballfollow->grabFrame(frame);
    QImage image1((unsigned char*)frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
    ui->outputLabel->setPixmap(QPixmap::fromImage(image1.rgbSwapped()));
}

void BallFollowingForm::on_testVideoButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory( this, "Choose image directory", "" );
    if ( !path.isEmpty() )
    {
        emit doTest( path );
    }
}
