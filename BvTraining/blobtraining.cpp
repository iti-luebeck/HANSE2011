#include "blobtraining.h"

#include <opencv/cxcore.h>
#include <opencv/highgui.h>

using namespace std;

BlobTraining::BlobTraining()
{
}

void BlobTraining::train(QList<int> frameList, QString videoFile)
{
    QList<Mat> featureList;
    QList<Mat> classesList;
    int totalFeatures = 0;

    VideoCapture vc(videoFile.toStdString());
    if (vc.isOpened())
    {
        namedWindow("Image", 1);
        namedWindow("Blob", 1);
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
    cvDestroyWindow("Blob");

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

            int key = waitKey(500);
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



