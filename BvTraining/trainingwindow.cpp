#include "trainingwindow.h"
#include "ui_trainingwindow.h"

#include <QFileDialog>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "imageprocessor.h"

using namespace cv;

TrainingWindow::TrainingWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TrainingWindow)
{
    ui->setupUi(this);

    videoFile = "../../front_sauce10.avi";
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
    QList<Mat> featureList;
    QList<Mat> classesList;
    int totalFeatures = 0;

    VideoCapture vc(videoFile.toStdString());
    if (vc.isOpened())
    {
        namedWindow("Image", 1);
        for (int i = 0; i < frameList.count(); i++)
        {
            int frameNr = frameList.at(i);
            vc.set(CV_CAP_PROP_POS_FRAMES, (double) frameList.at(i));

            Mat frame;
            vc >> frame;
            if (!frame.empty())
            {
                imshow("Image", frame);

                Mat thresh;
                Mat features;
                Mat classes;
                ip.threshold(frame, thresh);
                ip.features(thresh, features, classes);

                featureList.append(features);
                classesList.append(classes);
                totalFeatures += classes.rows;
            }
        }
    }
    vc.release();

    int k = 0;
    Mat featuresTemp(totalFeatures, 2, CV_32FC1, Scalar(0));
    Mat classesTemp(totalFeatures, 1, CV_32FC1, Scalar(0));
    for (int i = 0; i < featureList.count(); i++)
    {
        Mat f = featureList.at(i);
        int numFeatures = f.rows;
        featuresTemp.rowRange(k, k + numFeatures) = featuresTemp.rowRange(k, k + numFeatures) + f;

        Mat c = classesList.at(i);
        classesTemp.rowRange(k, k + numFeatures) = classesTemp.rowRange(k, k + numFeatures) + c;

        k += numFeatures;
    }

    CvMat *features = new CvMat(featuresTemp);
    CvMat *classes = new CvMat(classesTemp);
    //cvSet(classes, cvScalar(0));
    //cvmSet(classes, 0, 0, 1);

    svm = new SVMClassifier();
    svm->train(features, classes);

    svm->saveClassifier("bla.xml");
}

void TrainingWindow::on_loadButton_clicked()
{
    videoFile = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mpg *.divx)"));
}

void TrainingWindow::on_selectButton_clicked()
{
    VideoCapture vc(videoFile.toStdString());
    if (vc.isOpened())
    {
        frameList.clear();

        namedWindow("Press key to select image", 1);
        while (true)
        {
            Mat frame;
            vc >> frame;
            if (!frame.empty())
            {
                imshow("Press [s] to select image", frame);
                int key = waitKey(500);
                if (key == 'q')
                {
                    break;
                }
                else if (key == 's')
                {
                    frameList.append((int) vc.get(CV_CAP_PROP_POS_FRAMES));
                }
            }
        }

    }
    vc.release();
}

void TrainingWindow::on_testButton_clicked()
{
    svm = new SVMClassifier();
    svm->loadClassifier("bla.xml");

    VideoCapture vc(videoFile.toStdString());
    if (vc.isOpened())
    {
        namedWindow("Image", 1);
        for (;;)
        {
            Mat frame;
            vc >> frame;
            if (!frame.empty())
            {
                imshow("Image", frame);

                Mat thresh;
                Mat features;
                ip.threshold(frame, thresh);
                ip.features(thresh, features, svm);
            }

            waitKey(500);
        }
    }
    vc.release();
}
