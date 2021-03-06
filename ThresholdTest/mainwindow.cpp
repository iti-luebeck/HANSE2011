#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <QtCore>
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
    YB = NULL;
    binary = NULL;

    colorspace = 1;
    channel = 1;
    method = 1;
    inverted = false;

    lowThreshold = 0;
    highThreshold = 255;

    playing = false;
    count = 0;
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
        else if ( colorspace == 4 )
        {
            tempImage = YB;
        }


        int w = tempImage->height;
        int h = tempImage->width;

        if (method == 1) {
            cvThreshold( tempImage, binary, highThreshold, 255, CV_THRESH_TOZERO_INV );
            cvThreshold( binary, binary, lowThreshold, 255, CV_THRESH_BINARY );
            if (inverted) {
                cvNot(binary, binary);
            }
        } else if (method == 2) {
            if (inverted) {
                cvThreshold(tempImage, binary, 100, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
            } else {
                cvThreshold(tempImage, binary, 100, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
            }
        } else if (method == 3) {
            CvMat *samples = cvCreateMat(w*h, 1, CV_32F);
            CvMat *labels = cvCreateMat(w*h, 1, CV_32S);

            for (int i = 0; i < w; i++) {
                for (int j = 0; j < h; j++) {
                    cvmSet(samples, i*h + j, 0, cvGet2D(tempImage, i, j).val[0]);
                }
            }
            CvTermCriteria termcrit = cvTermCriteria(CV_TERMCRIT_ITER, 1000, 10);
            cvKMeans2(samples, 2, labels, termcrit);

            float s0 = 0.0f;
            for (int i = 0; i < w*h; i++) {
                if (cvGet2D(labels, i, 0).val[0] == 0) {
                    s0 = cvGet2D(samples, i, 0).val[0];
                    break;
                }
            }
            float s1 = 0.0f;
            for (int i = 0; i < w*h; i++) {
                if (cvGet2D(labels, i, 0).val[0] == 1) {
                    s1 = cvGet2D(samples, i, 0).val[0];
                    break;
                }
            }

            for (int i = 0; i < w; i++) {
                for (int j = 0; j < h; j++) {
                    if (inverted) {
                        if (s1 < s0) {
                            cvSet2D(binary, i, j, cvScalar(255*cvGet2D(labels, i*h + j, 0).val[0]));
                        } else {
                            cvSet2D(binary, i, j, cvScalar(255*(1 - cvGet2D(labels, i*h + j, 0).val[0])));
                        }
                    } else {
                        if (s1 > s0) {
                            cvSet2D(binary, i, j, cvScalar(255*cvGet2D(labels, i*h + j, 0).val[0]));
                        } else {
                            cvSet2D(binary, i, j, cvScalar(255*(1 - cvGet2D(labels, i*h + j, 0).val[0])));
                        }
                    }
                }
            }

            cvReleaseMat(&samples);
            cvReleaseMat(&labels);
        }

        IplImage *rgbBinary = cvCreateImage( cvGetSize( binary ), IPL_DEPTH_8U, 3 );
        IplImage *rgbChannel = cvCreateImage( cvGetSize( tempImage ), IPL_DEPTH_8U, 3 );

//        cvDilate( binary, binary, NULL, 20 );
//        cvErode( binary, binary, NULL, 20 );

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

        cvCvtColor( tempImage, rgbChannel, CV_GRAY2RGB );

        ui->rotationLabel->setText( QString( "theta = %1" ).arg( theta * 180 / CV_PI, 0, 'f', 1 ) );

        QImage rgb((unsigned char*)rgbImage->imageData, rgbImage->width, rgbImage->height, QImage::Format_RGB888);
        ui->originalLabel->setPixmap(QPixmap::fromImage(rgb));
        cvCvtColor(rgbImage, rgbImage, CV_RGB2BGR);
        cvSaveImage("threshold_rgb.jpg", rgbImage);
        cvCvtColor(rgbImage, rgbImage, CV_BGR2RGB);

        QImage cha((unsigned char*)rgbChannel->imageData, rgbChannel->width, rgbChannel->height, QImage::Format_RGB888);
        ui->channelLabel->setPixmap(QPixmap::fromImage(cha));
        cvCvtColor(rgbChannel, rgbChannel, CV_RGB2BGR);
        cvSaveImage("threshold_channel.jpg", rgbChannel);
        cvCvtColor(rgbChannel, rgbChannel, CV_BGR2RGB);

        QImage b((unsigned char*)rgbBinary->imageData, rgbBinary->width, rgbBinary->height, QImage::Format_RGB888);
        ui->label->setPixmap(QPixmap::fromImage(b));
        cvSaveImage("threshold_binary.jpg", rgbBinary);

        cvReleaseImage( &rgbBinary );
        cvReleaseImage( &rgbChannel );
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
    loadImage(fileName);
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

void MainWindow::on_bluyelRadiButton_clicked()
{
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

void MainWindow::on_playButton_clicked()
{
    if (playing) {
        playing = false;
    } else {
        QDir dir(QFileDialog::getExistingDirectory(this, QString("bla")));
        dir.setFilter( QDir::Files );
        files = dir.entryList();
        QStringList filters;
//        filters << "*.png";
        files = dir.entryList(filters, QDir::Files);
        for (int i = 0; i < files.size(); i++) {
            files[i] = dir.filePath(files[i]);
        }

        playing = true;
        count = 0;
        QTimer::singleShot(1000, this, SLOT(nextImage()));
    }
}

void MainWindow::nextImage()
{
    if (count < files.size()) {
        QString fileName = files[count];
        loadImage(fileName);

        count++;

        if (playing) {
            QTimer::singleShot(1000, this, SLOT(nextImage()));
        }
    } else {
        playing = false;
    }
}

void MainWindow::loadImage(QString fileName)
{
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
        if ( YB ) cvReleaseImage( &YB );
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

        YB = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );
        double maxVal = 0;
        double yellowMax = 0;
        double blueMax = 0;
        for (int i = 200; i < rgbImage->height; i++) {
            for (int j = 0; j < rgbImage->width - 50; j++) {
                double yellow = (cvGet2D(rgbImage, i, j).val[1] + cvGet2D(rgbImage, i, j).val[2]) / 2;
                double blue = cvGet2D(rgbImage, i, j).val[0];
                if (yellow / blue > maxVal) {
                    maxVal = yellow / blue;
                    yellowMax = yellow;
                    blueMax = blue;
                }
            }
        }
        for (int i = 0; i < rgbImage->height; i++) {
            for (int j = 0; j < rgbImage->width; j++) {
                double yellow = (cvGet2D(rgbImage, i, j).val[1] + cvGet2D(rgbImage, i, j).val[2]) / 2;
                double blue = cvGet2D(rgbImage, i, j).val[0];
                int val = 255.0 * (yellow / blue) / maxVal;
                if (val < 0) val = 0;
                if (val > 255) val = 255;
                cvSet2D(YB, i, j, cvScalar(val));
            }
        }

        binary = cvCreateImage( cvGetSize( rgbImage ), IPL_DEPTH_8U, 1 );

        refreshThreshold();
    }
}

void MainWindow::on_normalRadioButton_toggled(bool checked)
{
    if ( checked )
    {
        method = 1;
        refreshThreshold();
    }
}

void MainWindow::on_otsuRadioButton_toggled(bool checked)
{
    if ( checked )
    {
        method = 2;
        refreshThreshold();
    }
}

void MainWindow::on_kmeansRadioButton_toggled(bool checked)
{
    if ( checked )
    {
        method = 2;
        refreshThreshold();
    }
}

void MainWindow::on_bluyelRadiButton_toggled(bool checked)
{
    if ( checked )
    {
        colorspace = 4;
        refreshThreshold();
    }
}
