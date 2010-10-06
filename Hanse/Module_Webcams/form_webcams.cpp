#include "form_webcams.h"
#include "ui_form_webcams.h"
#include <Module_Webcams/module_webcams.h>
#include <opencv/highgui.h>
#include <Framework/dataloghelper.h>

Form_Webcams::Form_Webcams( Module_Webcams *cams, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_Webcams)
{
    ui->setupUi(this);
    this->cams = cams;

    leftFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    rightFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    bottomFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );

    refreshLists();

    QObject::connect( this, SIGNAL( changedSettings() ),
                      cams, SLOT( settingsChanged() ) );
//    QObject::connect( this, SIGNAL( captureTurnedOn() ),
//                      cams, SLOT( startCapture() ) );
//    QObject::connect( this, SIGNAL( captureTurnedOff() ),
//                      cams, SLOT( stopCapture() ) );
    QObject::connect( this, SIGNAL( showSettings(int) ),
                      cams, SLOT( showSettings(int) ) );

    count = 0;
    QObject::connect( &captureTimer, SIGNAL( timeout() ),
                      this, SLOT( captureWebcams() ) );

//    ui->checkBox->setChecked( true );
//    captureTimer.start( 500 );
    if ( cams->getSettings().value( "capture", true ).toBool() )
    {
        ui->checkBox->setChecked( true );
        captureTimer.start( 500 );
    }
    else
    {
        ui->checkBox->setChecked( false );
        captureTimer.stop();
        this->refreshFrames();
    }
}

Form_Webcams::~Form_Webcams()
{
    delete ui;
    cvReleaseImage( &leftFrame );
    cvReleaseImage( &rightFrame );
    cvReleaseImage( &bottomFrame );
}

void Form_Webcams::captureWebcams()
{
    QString dir = DataLogHelper::getLogDir();
    QDir d( dir );
    if ( !d.cd( "cam" ) )
    {
        d.mkdir( "cam" );
        d.cd( "cam" );
    }

    refreshFrames();

    char leftName[100];
    sprintf( leftName, d.absolutePath().append( "/left%04d.jpg" ).toStdString().c_str(), count );
    cvSaveImage( leftName, leftFrame );
    char rightName[100];
    sprintf( rightName, d.absolutePath().append( "/right%04d.jpg" ).toStdString().c_str(), count );
    cvSaveImage( rightName, rightFrame );
    char bottomName[100];
    sprintf( bottomName, d.absolutePath().append( "/bottom%04d.jpg" ).toStdString().c_str(), count );
    cvSaveImage( bottomName, bottomFrame );
    count++;
}

void Form_Webcams::changeEvent(QEvent *e)
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

void Form_Webcams::refreshLists()
{
    ui->leftBox->clear();
    ui->rightBox->clear();
    ui->bottomBox->clear();

    videoInput VI;
    int numCameras = VI.listDevices( true );
    for ( int i = 0; i < numCameras; i++ )
    {
        QString boxName = QString( "(%1) " ).arg( i );
        boxName.append( VI.getDeviceName( i ) );
        ui->leftBox->addItem( boxName );
        ui->rightBox->addItem( boxName );
        ui->bottomBox->addItem( boxName );
    }

    QSettings& settings = cams->getSettings();
    ui->leftBox->setCurrentIndex( settings.value( "leftID", 0 ).toInt() );
    ui->rightBox->setCurrentIndex( settings.value( "rightID", 1 ).toInt() );
    ui->bottomBox->setCurrentIndex( settings.value( "bottomID", 2 ).toInt() );
}

void Form_Webcams::on_applyButtn_clicked()
{
    QSettings& settings = cams->getSettings();
    settings.setValue( "leftID", ui->leftBox->currentIndex() );
    settings.setValue( "rightID", ui->rightBox->currentIndex() );
    settings.setValue( "bottomID", ui->bottomBox->currentIndex() );
    emit changedSettings();
}

void Form_Webcams::on_refreshButton_clicked()
{
    refreshFrames();
}

void Form_Webcams::refreshFrames()
{
    cams->grabLeft( leftFrame );
    cams->grabRight( rightFrame );
    cams->grabBottom( bottomFrame);

    QImage imageLeft( (unsigned char*)leftFrame->imageData, leftFrame->width, leftFrame->height,
                      QImage::Format_RGB888 );
    imageLeft = imageLeft.rgbSwapped();
    ui->leftLabel->setPixmap( QPixmap::fromImage( imageLeft ) );

    QImage imageRight( (unsigned char*)rightFrame->imageData, rightFrame->width, rightFrame->height,
                      QImage::Format_RGB888 );
    imageRight = imageRight.rgbSwapped();
    ui->rightLabel->setPixmap( QPixmap::fromImage( imageRight ) );

    QImage imageBottom( (unsigned char*)bottomFrame->imageData, bottomFrame->width, bottomFrame->height,
                      QImage::Format_RGB888 );
    imageBottom = imageBottom.rgbSwapped();
    ui->bottomLabel->setPixmap( QPixmap::fromImage( imageBottom ) );
}

void Form_Webcams::on_updateListButton_clicked()
{
    refreshLists();
}

void Form_Webcams::on_checkBox_clicked()
{
    cams->getSettings().setValue( "capture", ui->checkBox->isChecked() );
    if ( ui->checkBox->isChecked() )
    {
        captureTimer.start( 500 );
    }
    else
    {
        captureTimer.stop();
    }
}

void Form_Webcams::on_leftSettingsButton_clicked()
{
    emit showSettings( cams->getSettings().value( "leftID", 0 ).toInt() );
}

void Form_Webcams::on_rightSettingsButton_clicked()
{
    emit showSettings( cams->getSettings().value( "rightID", 1 ).toInt() );
}

void Form_Webcams::on_bottomSettingsButton_clicked()
{
    emit showSettings( cams->getSettings().value( "bottomID", 2 ).toInt() );
}
