#include "trainingwindow.h"
#include "ui_trainingwindow.h"

#include <QFileDialog>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "imageprocessor.h"
//#include "videoStream.h"
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
        surfTraining.train(frameList, videoFile);
    }
}

void TrainingWindow::on_loadButton_clicked()
{
    videoFile = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mpg *.divx)"));
}

void TrainingWindow::on_selectButton_clicked()
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
        surfTraining.test(videoFile);
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
