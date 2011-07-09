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

    ui->frontConnectCheckBox->setChecked(false);
    ui->bottomConnectCheckBox->setChecked(false);

    frontFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );
    bottomFrame.create( WEBCAM_HEIGHT, WEBCAM_WIDTH, CV_8UC3 );

    refreshLists();

    QObject::connect( this, SIGNAL( changedSettings() ),
                      cams, SLOT( settingsChanged() ) );
    QObject::connect(cams, SIGNAL(newFrontImage(cv::Mat)), this, SLOT(newFrontImage(cv::Mat)));
    QObject::connect(cams, SIGNAL(newBottomImage(cv::Mat)), this, SLOT(newBottomImage(cv::Mat)));
    frontCount = 0;
    bottomCount = 0;
}

Form_Webcams::~Form_Webcams()
{
    delete ui;
    frontFrame.release();
    bottomFrame.release();
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
    ui->frontBox->clear();
    ui->bottomBox->clear();

    std::vector<int> camsInd = this->cams->numOfCams();
    for (unsigned int i = 0; i < camsInd.size(); i++ )
    {
        QString boxName = QString::number(camsInd.at(i));
        ui->frontBox->addItem( boxName );
        ui->bottomBox->addItem( boxName );

        if(camsInd.at(i) == cams->getSettingsValue("frontID",0).toInt())
            ui->frontBox->setCurrentIndex(i);
        if(camsInd.at(i) == cams->getSettingsValue("bottomID",2).toInt())
            ui->bottomBox->setCurrentIndex(i);
    }
}

void Form_Webcams::newFrontImage(cv::Mat front)
{
    if (ui->captureCheckBox->isChecked()) {
        QImage imagefront((unsigned char*)front.data, front.cols, front.rows, QImage::Format_RGB888);
        ui->frontLabel->setPixmap( QPixmap::fromImage( imagefront ) );

        QString dir = DataLogHelper::getLogDir();
        QDir d(dir);
        if (!d.cd("cam")) {
            d.mkdir("cam");
            d.cd("cam");
        }

        char frontName[100];
        sprintf( frontName, d.absolutePath().append( "/front%04d.jpg" ).toStdString().c_str(), frontCount );
        IplImage* fronty = new IplImage(frontFrame);
        cvSaveImage( frontName, fronty );
        frontCount++;
    }
}

void Form_Webcams::newBottomImage(cv::Mat bottom)
{
    if (ui->captureCheckBox->isChecked()) {
        QImage imagebottom((unsigned char*)bottom.data, bottom.cols, bottom.rows, QImage::Format_RGB888);
        ui->bottomLabel->setPixmap( QPixmap::fromImage( imagebottom ) );

        QString dir = DataLogHelper::getLogDir();
        QDir d(dir);
        if (!d.cd("cam")) {
            d.mkdir("cam");
            d.cd("cam");
        }

        char bottomName[100];
        sprintf( bottomName, d.absolutePath().append( "/bottom%04d.jpg" ).toStdString().c_str(), bottomCount );
        IplImage* bottomy = new IplImage(bottomFrame);
        cvSaveImage( bottomName, bottomy );
        bottomCount++;
    }
}

void Form_Webcams::on_updateListButton_clicked()
{
    refreshLists();
}

void Form_Webcams::on_frontConnectCheckBox_clicked()
{
    if(ui->frontBox->count() == 0)
    {
        ui->frontErrorLabel->setText("Error, no Webcam Available");
        ui->frontConnectCheckBox->setChecked(false);
        return;
    }

    if(  (ui->bottomConnectCheckBox->isChecked() && (ui->bottomBox->currentIndex() == ui->frontBox->currentIndex())))
    {
        ui->frontErrorLabel->setText("The cam you selected is already in use");
        ui->frontConnectCheckBox->setChecked(false);
        return;
    }

    ui->frontErrorLabel->clear();
    cams->setSettingsValue("frontEnabled",ui->frontConnectCheckBox->isChecked());
    cams->setSettingsValue("frontID",ui->frontBox->currentText());
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

    if( (ui->frontConnectCheckBox->isChecked() && (ui->bottomBox->currentIndex() == ui->frontBox->currentIndex())))
    {
        ui->bottomErrorLabel->setText("The cam you selected is already in use");
        ui->bottomConnectCheckBox->setChecked(false);
        return;
    }

    ui->bottomErrorLabel->clear();
    cams->setSettingsValue("bottomEnabled",ui->bottomConnectCheckBox->isChecked());
    cams->setSettingsValue("bottomID",ui->bottomBox->currentText());
    emit changedSettings();
}
