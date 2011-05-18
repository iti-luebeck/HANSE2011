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

    ui->leftConnectCheckBox->setChecked(false);
    ui->rightConnectCheckBox->setChecked(false);
    ui->bottomConnectCheckBox->setChecked(false);

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

    refreshLists();

    QObject::connect( this, SIGNAL( changedSettings() ),
                      cams, SLOT( settingsChanged() ) );
//    QObject::connect( this, SIGNAL( captureTurnedOn() ),
//                      cams, SLOT( startCapture() ) );
//    QObject::connect( this, SIGNAL( captureTurnedOff() ),
//                      cams, SLOT( stopCapture() ) );
    count = 0;
    QObject::connect( &captureTimer, SIGNAL( timeout() ),
                      this, SLOT( captureWebcams() ) );

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

    std::vector<int> camsInd = this->cams->numOfCams();
    for (unsigned int i = 0; i < camsInd.size(); i++ )
    {
        QString boxName = QString::number(camsInd.at(i));
        ui->leftBox->addItem( boxName );
        ui->rightBox->addItem( boxName );
        ui->bottomBox->addItem( boxName );

        if(camsInd.at(i) == cams->getSettingsValue("leftID",0).toInt())
            ui->leftBox->setCurrentIndex(i);
        if(camsInd.at(i) == cams->getSettingsValue("RightID",1).toInt())
            ui->rightBox->setCurrentIndex(i);
        if(camsInd.at(i) == cams->getSettingsValue("bottomID",2).toInt())
            ui->bottomBox->setCurrentIndex(i);
    }
}

void Form_Webcams::on_applyButtn_clicked()
{
    cams->setSettingsValue( "leftID", ui->leftBox->currentText() );
    cams->setSettingsValue("rightID", ui->rightBox->currentText() );
    cams->setSettingsValue("bottomID", ui->bottomBox->currentText() );
//    cams->setSettingsValue( "leftID", ui->leftCamID->text() );
//    cams->setSettingsValue("rightID", ui->rightCamID->text() );
//    cams->setSettingsValue("bottomID", ui->bottomCamID->text() );

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
    ui->leftLabel->setPixmap( QPixmap::fromImage( imageLeft ) );

       QImage imageRight((unsigned char*)rightFrame.data, rightFrame.cols, rightFrame.rows, QImage::Format_RGB888);
    ui->rightLabel->setPixmap( QPixmap::fromImage( imageRight ) );

       QImage imageBottom((unsigned char*)bottomFrame.data, bottomFrame.cols, bottomFrame.rows, QImage::Format_RGB888);
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

void Form_Webcams::on_leftConnectCheckBox_clicked()
{
    if(ui->leftBox->count() == 0)
    {
        ui->leftErrorLabel->setText("Error, no Webcam Available");
        ui->leftConnectCheckBox->setChecked(false);
        return;
    }

    if( (ui->rightConnectCheckBox->isChecked() && (ui->rightBox->currentIndex() == ui->leftBox->currentIndex()))
        || (ui->bottomConnectCheckBox->isChecked() && (ui->bottomBox->currentIndex() == ui->leftBox->currentIndex())))
    {
        ui->leftErrorLabel->setText("The cam you selected is already in use");
        ui->leftConnectCheckBox->setChecked(false);
        return;
    }

    ui->leftErrorLabel->clear();
    cams->setSettingsValue("leftEnabled",ui->leftConnectCheckBox->isChecked());
    cams->setSettingsValue("leftFramerate",ui->leftFrameRateSlider->value());
    cams->setSettingsValue("leftID",ui->leftBox->currentText());
    emit changedSettings();
}

void Form_Webcams::on_rightConnectCheckBox_clicked()
{
    if(ui->rightBox->count() == 0)
    {
        ui->rightErrorLabel->setText("Error, no Webcam Available");
        ui->rightConnectCheckBox->setChecked(false);
        return;
    }


    if( (ui->leftConnectCheckBox->isChecked() && (ui->rightBox->currentIndex() == ui->leftBox->currentIndex()))
        || (ui->bottomConnectCheckBox->isChecked() && (ui->bottomBox->currentIndex() == ui->rightBox->currentIndex())))
    {
        ui->rightErrorLabel->setText("The cam you selected is already in use");
        ui->rightConnectCheckBox->setChecked(false);
        return;
    }

    ui->rightErrorLabel->clear();
    cams->setSettingsValue("rightEnabled",ui->rightConnectCheckBox->isChecked());
    cams->setSettingsValue("rightFramerate",ui->rightFrameRateSlider->value());
    cams->setSettingsValue("rightID",ui->rightBox->currentText());
    emit changedSettings();
}

void Form_Webcams::on_bottomConnectCheckBox_clicked()
{
    if(ui->bottomBox->count() == 0)
    {
        ui->bottomErrorLabel->setText("Error, no Webcam Available");
        ui->bottomConnectCheckBox->setChecked(false);
        return;
    }

    if( (ui->leftConnectCheckBox->isChecked() && (ui->bottomBox->currentIndex() == ui->leftBox->currentIndex()))
        || (ui->rightConnectCheckBox->isChecked() && (ui->bottomBox->currentIndex() == ui->rightBox->currentIndex())))
    {
        ui->bottomErrorLabel->setText("The cam you selected is already in use");
        ui->bottomConnectCheckBox->setChecked(false);
        return;
    }

    ui->bottomErrorLabel->clear();
    cams->setSettingsValue("bottomEnabled",ui->bottomConnectCheckBox->isChecked());
    cams->setSettingsValue("bottomFramerate",ui->bottomFrameRateSlider->value());
    cams->setSettingsValue("bottomID",ui->bottomBox->currentText());
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
