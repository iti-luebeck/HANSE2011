#include "form_visualslam.h"
#include "ui_form_visualslam.h"
#include <QMessageBox>

Form_VisualSLAM::Form_VisualSLAM( Module_VisualSLAM *visualSlam, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_VisualSLAM)
{
    ui->setupUi(this);
    this->visualSlam = visualSlam;
    QObject::connect( this, SIGNAL( settingsChanged(double,double,double) ),
                      visualSlam, SLOT( changeSettings(double,double,double) ) );
    QObject::connect( visualSlam, SIGNAL(dataChanged(RobotModule*)),
                      this, SLOT(updateCams(RobotModule*)),
                      Qt::DirectConnection );

    ui->observationEdit->setText( QString("%1").arg( visualSlam->getObservationVariance() ) );
    ui->translationEdit->setText( QString("%1").arg( visualSlam->getTranslationVariance() ) );
    ui->rotationEdit->setText( QString("%1").arg( visualSlam->getRotationVariance() ) );

    videoInput VI;
    int numCameras = VI.listDevices( true );
    for ( int i = 0; i < numCameras; i++ )
    {
        QString boxName = QString( "(%1) " ).arg( i );
        boxName.append( VI.getDeviceName( i ) );
        ui->leftCameraBox->addItem( boxName );
        ui->rightCameraBox->addItem( boxName );
    }

    QSettings& settings = visualSlam->getSettings();
    ui->leftCameraBox->setCurrentIndex(
            settings.value( QString( "left_camera" ), VSLAM_CAMERA_LEFT ).toInt() );
    ui->rightCameraBox->setCurrentIndex(
            settings.value( QString( "right_camera" ), VSLAM_CAMERA_RIGHT ).toInt() );
    ui->checkBox->setChecked( settings.value( QString( "capture" ), false ).toBool() );
    ui->noSLAMBox->setChecked( settings.value( "no_slam", true ).toBool() );
}

Form_VisualSLAM::~Form_VisualSLAM()
{
    delete ui;
}

void Form_VisualSLAM::changeEvent(QEvent *e)
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

void Form_VisualSLAM::on_startButton_clicked()
{
    visualSlam->setEnabled( true );
}

void Form_VisualSLAM::on_stopButton_clicked()
{
    visualSlam->setEnabled( false );
}

void Form_VisualSLAM::on_resetButton_clicked()
{
    visualSlam->reset();;
}

void Form_VisualSLAM::on_applyButton_clicked()
{
    bool ok;
    double v_observation, v_translation, v_rotation;

    v_observation = ui->observationEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect observation variance." );
        msgBox.exec();
        return;
    }

    v_translation = ui->translationEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect observation variance." );
        msgBox.exec();
        return;
    }

    v_rotation = ui->rotationEdit->text().toDouble( &ok );
    if ( !ok )
    {
        QMessageBox msgBox;
        msgBox.setText( "Incorrect rotation variance." );
        msgBox.exec();
        return;
    }

    settingsChanged( v_observation, v_translation, v_rotation );

    int device1 = ui->leftCameraBox->currentIndex();
    int device2 = ui->rightCameraBox->currentIndex();

    if ( device1 == device2 )
    {
        QMessageBox msgBox;
        msgBox.setText( "Left and right Cameras must be different." );
        msgBox.exec();
        return;
    }
    QSettings& settings = visualSlam->getSettings();
    settings.setValue( QString( "left_camera"), device1 );
    settings.setValue( QString( "right_camera"), device2 );
    settings.setValue( "no_slam", ui->noSLAMBox->isChecked() );
}

void Form_VisualSLAM::on_checkBox_clicked()
{
    QSettings& settings = visualSlam->getSettings();
    settings.setValue( QString( "capture" ), ui->checkBox->isChecked() );
}

void Form_VisualSLAM::updateCams( RobotModule * )
{
    IplImage *frame1 = visualSlam->getFrame( 0 );
    QImage image1((unsigned char*)frame1->imageData, frame1->width, frame1->height, QImage::Format_Indexed8);
    ui->leftLabel->setPixmap(QPixmap::fromImage(image1));
    IplImage *frame2 = visualSlam->getFrame( 1 );
    QImage image2((unsigned char*)frame2->imageData, frame2->width, frame2->height, QImage::Format_Indexed8);
    ui->rightLabel->setPixmap(QPixmap::fromImage(image2));
}
