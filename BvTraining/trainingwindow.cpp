#include "trainingwindow.h"
#include "ui_trainingwindow.h"

#include <QFileDialog>
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "imageprocessor.h"
using namespace cv;

TrainingWindow::TrainingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrainingWindow)
{
    ui->setupUi(this);

    videoFile = "../../front_sauce10.avi";
    blobTraining.load("bla.xml");
}

TrainingWindow::~TrainingWindow()
{
    delete ui;
}

void TrainingWindow::changeEvent(QEvent *e)
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

void TrainingWindow::on_trainButton_clicked()
{
    if (ui->blobRadioButton->isChecked())
    {
        blobTraining.train(frameList, videoFile);
    }
    else if (ui->surfRadioButton->isChecked())
    {
        surfTraining.train(frameList, videoFile, ui->dirBox->isChecked());
    }
}

void TrainingWindow::on_loadButton_clicked()
{
    if ( ui->dirBox->isChecked() )
    {
        videoFile = QFileDialog::getExistingDirectory( this, QString("Choose directory with images"), "" );
    }
    else
    {
        videoFile = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mpg *.divx)"));
    }
}

void TrainingWindow::on_selectButton_clicked()
{
    if ( ui->dirBox->isChecked() )
    {
        QDir dir( videoFile );
        dir.setFilter( QDir::Files );
        QStringList filters;
        filters << "*.jpg";
        dir.setNameFilters( filters );
        QStringList files = dir.entryList();
        Mat frame;
        frameList.clear();

        namedWindow("Press [s] to select image", 0);
        for ( int i = 0; i < files.count(); i++ )
        {
            QString filePath = videoFile;
            filePath.append( "/" );
            filePath.append( files[i] );

            frame = imread( filePath.toStdString() );
            bool check = false;
            if ( !frame.empty() )
            {
                imshow("Press [s] to select image", frame);
                int key = waitKey(500);
                switch (key)
                {
                case 'q':
                    check = true;
                    break;
                case 's':
                    frameList.append( i );
                    break;
                case '+':
                    i += 10;
                    break;
                case '-':
                    i -= 10;
                    break;
                }
            }

            if ( check )
                break;
        }
        cvDestroyWindow("Press [s] to select image");
    }
    else
    {
        Mat frame;
        VideoCapture vc(videoFile.toStdString());
        if (vc.isOpened())
        {
            frameList.clear();

            namedWindow("Press [s] to select image", 0);
            bool running = true;
            while (running)
            {
                vc >> frame;
                if (!frame.empty())
                {
                    imshow("Press [s] to select image", frame);
                    int key = waitKey(500);
                    switch (key)
                    {
                    case 'q':
                        running = false;
                        break;
                    case 's':
                        frameList.append((int) vc.get(CV_CAP_PROP_POS_FRAMES));
                        break;
                    case '+':
                        vc.set(CV_CAP_PROP_POS_FRAMES, vc.get(CV_CAP_PROP_POS_FRAMES) + 10);
                        break;
                    case '-':
                        vc.set(CV_CAP_PROP_POS_FRAMES, vc.get(CV_CAP_PROP_POS_FRAMES) - 10);
                        break;
                    }
                }
            }
        }
        vc.release();
        cvDestroyWindow("Press [s] to select image");
    }
}

void TrainingWindow::on_testButton_clicked()
{
    if (ui->blobRadioButton->isChecked())
    {
        blobTraining.test(videoFile);
    }
    else if (ui->surfRadioButton->isChecked())
    {
        bool ok;
        double thresh = ui->threshEdit->text().toDouble(&ok);
        if (ok)
        {
            surfTraining.setThresh(thresh);
        }
        else
        {
            qDebug("Threshold ist kein double.");
            surfTraining.setThresh(0.7);
        }
        surfTraining.test(videoFile, ui->dirBox->isChecked());
    }
}

void TrainingWindow::on_loadBlobButton_clicked()
{
    QString loadFile = QFileDialog::getOpenFileName(this, tr("Load Classifier"), "", tr("Classifier Files (*.xml)"));
    if (ui->blobRadioButton->isChecked())
    {
        blobTraining.load(loadFile);
    }
    else if (ui->surfRadioButton->isChecked())
    {
        surfTraining.load(loadFile);
    }
}

void TrainingWindow::on_saveBlobButton_clicked()
{
    QString saveFile = QFileDialog::getSaveFileName(this, tr("Save Classifier"), "", tr("Classifier Files (*.xml)"));
    if (ui->blobRadioButton->isChecked())
    {
        blobTraining.save(saveFile);
    }
    else if (ui->surfRadioButton->isChecked())
    {
        surfTraining.save(saveFile);
    }
}

void TrainingWindow::on_streamButton_clicked()
{
    int webCamID = ui->webcamID->text().toInt();
    surfTraining.liveTest(webCamID);
}
