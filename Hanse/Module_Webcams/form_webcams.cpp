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

    ui->leftConnectCheckBox->setChecked(cams->getSettingsValue("leftEnabled",true).toBool());
    ui->rightConnectCheckBox->setChecked(cams->getSettingsValue("rightEnabled",true).toBool());
    ui->bottomConnectCheckBox->setChecked(cams->getSettingsValue("bottomEnabled",true).toBool());

    ui->leftCamID->setText(cams->getSettingsValue("leftID").toString());
    ui->rightCamID->setText(cams->getSettingsValue("rightID").toString());
    ui->bottomCamID->setText(cams->getSettingsValue("bottomID").toString());

    int pos = cams->getSettingsValue("leftFramerate",5).toInt();
    ui->leftFrameRateSlider->setValue(pos/5);
    ui->leftFramerateLabel->setText("Framerate: " +QString::number(pos)
                                    +" ("+QString::number(pos)+")");
    pos = cams->getSettingsValue("rightFramerate",5).toInt();
    ui->rightFrameRateSlider->setValue(pos/5);
    ui->rightFramerateLabel->setText("Framerate: " +QString::number(pos)
                                     +" ("+QString::number(pos)+")");
    pos = cams->getSettingsValue("bottomFramerate",5).toInt();
    ui->bottomFrameRateSlider->setValue(pos/5);
    ui->bottomFramerateLabel->setText("Framerate: " +QString::number(pos)
                                      +" ("+QString::number(pos)+")");

    leftFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    rightFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    bottomFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );

//    leftFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
//    rightFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );
//    bottomFrame = cvCreateImage( cvSize( WEBCAM_WIDTH, WEBCAM_HEIGHT ), IPL_DEPTH_8U, 3 );

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
    if ( cams->getSettingsValue( "capture", false ).toBool() )
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
    leftFrame.release();
    rightFrame.release();
    bottomFrame.release();
//    cvReleaseImage( &leftFrame );
//    cvReleaseImage( &rightFrame );
//    cvReleaseImage( &bottomFrame );
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

    if(this->cams->getSettingsValue("leftEnabled").toBool())
    {
        char leftName[100];
        sprintf( leftName, d.absolutePath().append( "/left%04d.jpg" ).toStdString().c_str(), count );
        IplImage* lefty = new IplImage(leftFrame);
        cvSaveImage( leftName, lefty );
    }
    if(this->cams->getSettingsValue("rightEnabled").toBool())
    {
        char rightName[100];
        sprintf( rightName, d.absolutePath().append( "/right%04d.jpg" ).toStdString().c_str(), count );
        IplImage* righty = new IplImage(rightFrame);
        cvSaveImage( rightName, righty );
    }
    if(this->cams->getSettingsValue("bottomEnabled").toBool())
    {
        char bottomName[100];
        sprintf( bottomName, d.absolutePath().append( "/bottom%04d.jpg" ).toStdString().c_str(), count );
        IplImage* bottomy = new IplImage(bottomFrame);
        cvSaveImage( bottomName, bottomy );
    }
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

//    videoInput VI;
    int numCameras = 3;//this->cams->numOfCams(); // VI.listDevices( true );
    for ( int i = 0; i < numCameras; i++ )
    {
        QString boxName = QString( "(%1) " ).arg( i );
//        boxName.append( VI.getDeviceName( i ) );
        boxName.append("Camera");
        ui->leftBox->addItem( boxName );
        ui->rightBox->addItem( boxName );
        ui->bottomBox->addItem( boxName );
    }

//    QSettings& settings = cams->getSettings();
    ui->leftBox->setCurrentIndex( cams->getSettingsValue( "leftID", 0 ).toInt() );
    ui->rightBox->setCurrentIndex( cams->getSettingsValue( "rightID", 1 ).toInt() );
    ui->bottomBox->setCurrentIndex( cams->getSettingsValue( "bottomID", 2 ).toInt() );
}

void Form_Webcams::on_applyButtn_clicked()
{
//    QSettings& settings = cams->getSettings();
//    cams->setSettingsValue( "leftID", ui->leftBox->currentIndex() );
//    cams->setSettingsValue("rightID", ui->rightBox->currentIndex() );
//    cams->setSettingsValue("bottomID", ui->bottomBox->currentIndex() );
    cams->setSettingsValue( "leftID", ui->leftCamID->text() );
    cams->setSettingsValue("rightID", ui->rightCamID->text() );
    cams->setSettingsValue("bottomID", ui->bottomCamID->text() );

    cams->setSettingsValue("leftEnabled",ui->leftConnectCheckBox->isChecked());
    cams->setSettingsValue("rightEnabled",ui->rightConnectCheckBox->isChecked());
    cams->setSettingsValue("bottomEnabled",ui->bottomConnectCheckBox->isChecked());

    int pos = ui->leftFrameRateSlider->value() *5;
    cams->setSettingsValue("leftFramerate",pos);
    ui->leftFramerateLabel->setText("Framerate: " +QString::number(pos)
                                    +" ("+QString::number(pos)+")");

    pos = ui->rightFrameRateSlider->value() *5;
    cams->setSettingsValue("rightFramerate",pos);
    ui->rightFramerateLabel->setText("Framerate: " +QString::number(pos)
                                     +" ("+QString::number(pos)+")");

    pos = ui->bottomFrameRateSlider->value() * 5;
    cams->setSettingsValue("bottomFramerate",pos);
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

    QImage imageLeft((unsigned char*)leftFrame.data, leftFrame.cols, leftFrame.rows, QImage::Format_RGB888);
//    imageLeft = imageLeft.rgbSwapped();
    ui->leftLabel->setPixmap( QPixmap::fromImage( imageLeft ) );

       QImage imageRight((unsigned char*)rightFrame.data, rightFrame.cols, rightFrame.rows, QImage::Format_RGB888);
//       imageRight = imageRight.rgbSwapped();
    ui->rightLabel->setPixmap( QPixmap::fromImage( imageRight ) );

//    QImage imageBottom( (unsigned char*)bottomFrame->imageData, bottomFrame->width, bottomFrame->height,
//                      QImage::Format_RGB888 );
       QImage imageBottom((unsigned char*)bottomFrame.data, bottomFrame.cols, bottomFrame.rows, QImage::Format_RGB888);
//    imageBottom = imageBottom.rgbSwapped();
    ui->bottomLabel->setPixmap( QPixmap::fromImage( imageBottom ) );
}

void Form_Webcams::on_updateListButton_clicked()
{
    refreshLists();
}

void Form_Webcams::on_checkBox_clicked()
{
    cams->setSettingsValue( "capture", ui->checkBox->isChecked() );
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
    emit showSettings( cams->getSettingsValue( "leftID", 0 ).toInt() );
}

void Form_Webcams::on_rightSettingsButton_clicked()
{
    emit showSettings( cams->getSettingsValue( "rightID", 1 ).toInt() );
}

void Form_Webcams::on_bottomSettingsButton_clicked()
{
    emit showSettings( cams->getSettingsValue("bottomID", 2 ).toInt() );
}

void Form_Webcams::on_leftConnectCheckBox_clicked()
{
    cams->setSettingsValue("leftEnabled",ui->leftConnectCheckBox->isChecked());
    cams->setSettingsValue("leftFramerate",ui->leftFrameRateSlider->value());
    cams->setSettingsValue("leftID",ui->leftCamID->text());
    emit changedSettings();
}

void Form_Webcams::on_rightConnectCheckBox_clicked()
{
    cams->setSettingsValue("rightEnabled",ui->rightConnectCheckBox->isChecked());
    cams->setSettingsValue("rightFramerate",ui->rightFrameRateSlider->value());
    cams->setSettingsValue("rightID",ui->rightCamID->text());
    emit changedSettings();
}

void Form_Webcams::on_bottomConnectCheckBox_clicked()
{
    cams->setSettingsValue("bottomEnabled",ui->bottomConnectCheckBox->isChecked());
    cams->setSettingsValue("bottomFramerate",ui->bottomFrameRateSlider->value());
    cams->setSettingsValue("bottomID",ui->bottomCamID->text());
    emit changedSettings();
}

void Form_Webcams::on_leftFrameRateSlider_sliderMoved(int position)
{
    position *= 5;
    ui->leftFramerateLabel->setText("Framerate: " +QString::number(position)
                                    +"("+QString::number(cams->getSettingsValue("leftFramerate").toInt())
                                                         +")");
}


void Form_Webcams::on_rightFrameRateSlider_sliderMoved(int position)
{
    position *= 5;
    ui->rightFramerateLabel->setText("Framerate: " +QString::number(position)
                                    +"("+QString::number(cams->getSettingsValue("rightFramerate").toInt())
                                                         +")");

}

void Form_Webcams::on_bottomFrameRateSlider_sliderMoved(int position)
{
    position *= 5;
    ui->bottomFramerateLabel->setText("Framerate: " +QString::number(position)
                                    +"("+QString::number(cams->getSettingsValue("bottomFramerate").toInt())
                                                         +")");
}
