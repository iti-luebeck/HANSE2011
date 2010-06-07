#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    rgbImage = NULL;
    hsvImage = NULL;
    grayscaleImage = NULL;
    R = NULL;
    G = NULL;
    B = NULL;
    H = NULL;
    S = NULL;
    V = NULL;
    binary = NULL;

    colorspace = 1;
    channel = 1;
    inverted = false;

    lowThreshold = 0;
    highThreshold = 255;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::refreshThreshold()
{
    if ( rgbImage != NULL )
    {
        IplImage *tempImage;

        if ( colorspace == 1 )
        {
            tempImage = grayscaleImage;
        }
        else if ( colorspace == 2 )
        {
            if ( channel == 1 )
            {
                tempImage = R;
            }
            else if ( channel == 2 )
            {
                tempImage = G;
            }
            else if ( channel == 3 )
            {
                tempImage = B;
            }
        }
        else if ( colorspace == 3 )
        {
            if ( channel == 1 )
            {
                tempImage = H;
            }
            else if ( channel == 2 )
            {
                tempImage = S;
            }
            else if ( channel == 3 )
            {
                tempImage = V;
            }
        }

        cvEqualizeHist( tempImage, tempImage );

        cvThreshold( tempImage, binary, highThreshold, 255, CV_THRESH_TOZERO_INV );
        cvThreshold( binary, binary, lowThreshold, 255, CV_THRESH_BINARY );

        if ( inverted )
        {
            cvNot( binary, binary );
        }

        IplImage *rgbBinary = cvCreateImage( cvGetSize( binary ), IPL_DEPTH_8U, 3 );
        //cvPyrMeanShiftFiltering( rgbImage, rgbBinary, 20, 40, 2 );

        cvDilate( binary, binary, NULL, 5 );
        cvErode( binary, binary, NULL, 10 );

        CvMoments M;
        cvMoments( binary, &M, 1 );

        double m00 = cvGetSpatialMoment( &M, 0, 0 );
        double m10 = cvGetSpatialMoment( &M, 1, 0 ) / m00;
        double m01 = cvGetSpatialMoment( &M, 0, 1 ) / m00;
        double mu11 = cvGetCentralMoment( &M, 1, 1 ) / m00;
        double mu20 = cvGetCentralMoment( &M, 2, 0 ) / m00;
        double mu02 = cvGetCentralMoment( &M, 0, 2 ) / m00;
        // moments( binary, m10, m01, mu11, mu02, mu20 );
        double theta = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );

        cvCvtColor( binary, rgbBinary, CV_GRAY2RGB );
        cvLine( rgbBinary, cvPoint( m10, m01 ),
                cvPoint( m10 + cos(theta)*200, m01 + sin(theta)*200 ),
                cvScalar( 255, 0, 0 ), 4, CV_FILLED );

        ui->rotationLabel->setText( QString( "theta = %1" ).arg( theta * 180 / CV_PI, 0, 'f', 1 ) );

        QImage rgb((unsigned char*)rgbImage->imageData, rgbImage->width, rgbImage->height, QImage::Format_RGB888);
        ui->originalLabel->setPixmap(QPixmap::fromImage(rgb));

        QImage b((unsigned char*)rgbBinary->imageData, rgbBinary->width, rgbBinary->height, QImage::Format_RGB888);
        ui->label->setPixmap(QPixmap::fromImage(b));

        cvReleaseImage( &rgbBinary );
    }
}

void MainWindow::moments( IplImage *image, double &m10, double &m01, double &mu11, double &mu02, double &mu20 )
{
    m10 = 0;
    m01 = 0;
    mu11 = 0;
    mu02 = 0;
    mu20 = 0;

    double m00 = 0;

    for ( int i = 0; i < image->height; i++ )
    {
        for ( int j = 0; j < image->width; j++ )
        {
            if ( cvGet2D( image, i, j ).val[0] > 0 )
            {
                m00++;
                m10 += i;
                m01 += j;
            }
        }
    }

    m10 = m10 / m00;
    m01 = m01 / m00;

    for ( int i = 0; i < image->height; i++ )
    {
        for ( int j = 0; j < image->width; j++ )
        {
            if ( cvGet2D( image, i, j ).val[0] > 0 )
            {
                mu11 += ( i - m10 ) * ( j - m01 );
                mu20 += ( i - m10 ) * ( i - m10 );
                mu02 += ( j - m01 ) * ( j - m01 );
            }
        }
    }
}

void MainWindow::on_loadButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image Files (*.jpg *.png *.bmp)"));

    if ( QFile::exists( fileName ) )
    {
        if ( rgbImage ) cvReleaseImage( &rgbImage );
        if ( hsvImage ) cvReleaseImage( &hsvImage );
        if ( grayscaleImage ) cvReleaseImage( &grayscaleImage );
        if ( R ) cvReleaseImage( &R );
        if ( G ) cvReleaseImage( &G );
        if ( B ) cvReleaseImage( &B );
        if ( H ) cvReleaseImage( &H );
        if ( S ) cvReleaseImage( &S );
        if ( V ) cvReleaseImage( &V );
        if ( binary ) cvReleaseImage( &binary );

        rgbImage = cvLoadImage( fileName.toStdString().c_str(), 1 );
        cvCvtColor( rgbImage, rgbImage, CV_BGR2RGB );

        grayscaleImage = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        cvCvtColor( rgbImage, grayscaleImage, CV_RGB2GRAY );

        R = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        G = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        B = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        cvSplit( rgbImage, R, G, B, NULL );

        hsvImage = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 3 );
        cvCvtColor( rgbImage, hsvImage, CV_RGB2HSV );

        H = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        S = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        V = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        cvSplit( hsvImage, H, S, V, NULL );

        binary = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );

        refreshThreshold();
    }
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    lowThreshold = position;
    ui->lowLabel->setText( QString( "%1" ).arg( position ) );
    refreshThreshold();
}

void MainWindow::on_horizontalSlider_2_sliderMoved(int position)
{
    highThreshold = position;
    ui->highLabel->setText( QString( "%1" ).arg( position ) );
    refreshThreshold();
}

void MainWindow::on_grayRadioButton_toggled(bool checked)
{
    if ( checked )
    {
        colorspace = 1;
        refreshThreshold();
    }
}

void MainWindow::on_rgbRadioButton_toggled(bool checked)
{
    if ( checked )
    {
        colorspace = 2;
        refreshThreshold();
    }
}

void MainWindow::on_hsvRadioButton_toggled(bool checked)
{
    if ( checked )
    {
        colorspace = 3;
        refreshThreshold();
    }
}

void MainWindow::on_channel1RadioButton_toggled(bool checked)
{
    if ( checked )
    {
        channel = 1;
        refreshThreshold();
    }
}

void MainWindow::on_channel2RadioButton_toggled(bool checked)
{
    if ( checked )
    {
        channel = 2;
        refreshThreshold();
    }
}

void MainWindow::on_channel3RadioButton_toggled(bool checked)
{
    if ( checked )
    {
        channel = 3;
        refreshThreshold();
    }
}

void MainWindow::on_invertCheckBox_toggled(bool checked)
{
    inverted = checked;
    refreshThreshold();
}
