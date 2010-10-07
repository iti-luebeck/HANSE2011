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

    ui->leftConnectCheckBox->setChecked(cams->getSettings().value("leftEnabled",true).toBool());
    ui->rightConnectCheckBox->setChecked(cams->getSettings().value("rightEnabled",true).toBool());
    ui->bottomConnectCheckBox->setChecked(cams->getSettings().value("bottomEnabled",true).toBool());

    int pos = cams->getSettings().value("leftFramerate",5).toInt();
    ui->leftFrameRateSlider->setValue(pos);
    ui->leftFramerateLabel->setText("Framerate: " +QString::number(pos)
                                    +" ("+QString::number(pos)+")");
    pos = cams->getSettings().value("rightFramerate",5).toInt();
    ui->rightFrameRateSlider->setValue(pos);
    ui->rightFramerateLabel->setText("Framerate: " +QString::number(pos)
                                     +" ("+QString::number(pos)+")");
    pos = cams->getSettings().value("bottomFramerate",5).toInt();
    ui->bottomFrameRateSlider->setValue(pos);
    ui->bottomFramerateLabel->setText("Framerate: " +QString::number(pos)
                                      +" ("+QString::number(pos)+")");

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

    settings.setValue("leftEnabled",ui->leftConnectCheckBox->isChecked());
    settings.setValue("rightEnabled",ui->rightConnectCheckBox->isChecked());
    settings.setValue("bottomEnabled",ui->bottomConnectCheckBox->isChecked());

    int pos = ui->leftFrameRateSlider->value();
    settings.setValue("leftFramerate",pos);
    ui->leftFramerateLabel->setText("Framerate: " +QString::number(pos)
                                    +" ("+QString::number(pos)+")");

    pos = ui->rightFrameRateSlider->value();
    settings.setValue("rightFramerate",pos);
    ui->rightFramerateLabel->setText("Framerate: " +QString::number(pos)
                                     +" ("+QString::number(pos)+")");

    pos = ui->bottomFrameRateSlider->value();
    settings.setValue("bottomFramerate",pos);
    ui->bottomFramerateLabel->setText("Framerate: " +QString::number(pos)
                                      +" ("+QString::number(pos)+")");


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

void Form_Webcams::on_leftConnectCheckBox_clicked()
{
    cams->getSettings().setValue("leftEnabled",ui->leftConnectCheckBox->isChecked());
    cams->getSettings().setValue("leftFramerate",ui->leftFrameRateSlider->value());
    emit changedSettings();
}

void Form_Webcams::on_rightConnectCheckBox_clicked()
{
    cams->getSettings().setValue("rightEnabled",ui->rightConnectCheckBox->isChecked());
    cams->getSettings().setValue("rightFramerate",ui->rightFrameRateSlider->value());
    emit changedSettings();
}

void Form_Webcams::on_bottomConnectCheckBox_clicked()
{
    cams->getSettings().setValue("bottomEnabled",ui->bottomConnectCheckBox->isChecked());
    cams->getSettings().setValue("bottomFramerate",ui->bottomFrameRateSlider->value());
    emit changedSettings();
}

void Form_Webcams::on_leftFrameRateSlider_sliderMoved(int position)
{
    ui->leftFramerateLabel->setText("Framerate: " +QString::number(position)
                                    +"("+QString::number(cams->getSettings().value("leftFramerate").toInt())
                                                         +")");
}


void Form_Webcams::on_rightFrameRateSlider_sliderMoved(int position)
{
    ui->rightFramerateLabel->setText("Framerate: " +QString::number(position)
                                    +"("+QString::number(cams->getSettings().value("rightFramerate").toInt())
                                                         +")");

}

void Form_Webcams::on_bottomFrameRateSlider_sliderMoved(int position)
{
    ui->bottomFramerateLabel->setText("Framerate: " +QString::number(position)
                                    +"("+QString::number(cams->getSettings().value("bottomFramerate").toInt())
                                                         +")");
}
