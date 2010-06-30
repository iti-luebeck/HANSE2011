#include "form_webcams.h"
#include "ui_form_webcams.h"
#include <Module_Webcams/module_webcams.h>

Form_Webcams::Form_Webcams( Module_Webcams *cams, QWidget *parent ) :
    QWidget(parent),
    ui(new Ui::Form_Webcams)
{
    ui->setupUi(this);
    this->cams = cams;

    leftFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    rightFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    bottomFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
    
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

    QObject::connect( this, SIGNAL( changedSettings() ),
                      cams, SLOT( settingsChanged() ) );
}

Form_Webcams::~Form_Webcams()
{
    delete ui;
    cvReleaseImage( &leftFrame );
    cvReleaseImage( &rightFrame );
    cvReleaseImage( &bottomFrame );
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
    cams->grabLeft( leftFrame );
    cams->grabRight( rightFrame );
    cams->grabBottom( bottomFrame);

    QImage imageLeft( (unsigned char*)leftFrame->imageData, leftFrame->width, leftFrame->height,
                      QImage::Format_RGB888 );
    ui->leftLabel->setPixmap( QPixmap::fromImage( imageLeft ) );

    QImage imageRight( (unsigned char*)rightFrame->imageData, rightFrame->width, rightFrame->height,
                      QImage::Format_RGB888 );
    ui->rightLabel->setPixmap( QPixmap::fromImage( imageRight ) );

    QImage imageBottom( (unsigned char*)bottomFrame->imageData, bottomFrame->width, bottomFrame->height,
                      QImage::Format_RGB888 );
    ui->bottomLabel->setPixmap( QPixmap::fromImage( imageBottom ) );
}
