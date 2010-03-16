#include "blobtraining.h"

#include <cxcore.h>
#include <highgui.h>

using namespace std;

BlobTraining::BlobTraining()
{
}

void BlobTraining::select(QString videoFile)
{
    Mat frame;
    VideoCapture vc(videoFile.toStdString());
    if (vc.isOpened())
    {
        frameList.clear();

        namedWindow("Press [s] to select image", 1);
        while (true)
        {
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
    cvDestroyWindow("Press [s] to select image");
}

void BlobTraining::train(QString videoFile)
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
    cvDestroyWindow("Image");

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

    svm.train(features, classes);
}

void BlobTraining::test(QString videoFile)
{
    VideoCapture vc(videoFile.toStdString());
    if (vc.isOpened())
    {
        namedWindow("Image", 1);
        namedWindow("Klassifikation", 1);
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

            int key = waitKey(200);
            if (key == 'q') break;
        }
    }
    vc.release();
    cvDestroyWindow("Image");
    cvDestroyWindow("Klassifikation");
}

void BlobTraining::load(QString loadFile)
{
    svm.loadClassifier(loadFile.toStdString().c_str());
}

void BlobTraining::save(QString saveFile)
{
    svm.saveClassifier(saveFile.toStdString().c_str());
}



