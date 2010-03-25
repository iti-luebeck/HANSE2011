#include "surftraining.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "helpers.h"
#include "surf/surflib.h"
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

SurfTraining::SurfTraining()
{
}

void SurfTraining::train(QList<int> frameList, QString videoFile)
{
    QList<Mat> featureList;
    SURF surf(500.0);

    VideoCapture vc(videoFile.toStdString());
    if (vc.isOpened())
    {
        namedWindow("Image", 1);
        for (int i = 0; i < frameList.count(); i++)
        {
            vc.set(CV_CAP_PROP_POS_FRAMES, (double) frameList.at(i));

            Mat frame;
            Mat frameGray;
            vc >> frame;
            if (!frame.empty())
            {
                vector<KeyPoint> keyPoints;
                vector<float> descriptors;

                Helpers::convertBGR2Gray(frame, frameGray);

                surf(frameGray, Mat::ones(frameGray.size(), CV_8UC1), keyPoints, descriptors);

                for (int i = 0; i < keyPoints.size(); i++)
                {
                    Mat temp = frame.clone();
                    circle(temp, Point(keyPoints[i].pt.x, keyPoints[i].pt.y),  keyPoints[i].size, Scalar(0,0,255), 1, CV_FILLED);
                    circle(temp, Point(keyPoints[i].pt.x, keyPoints[i].pt.y),  2, Scalar(0,0,255), 2, CV_FILLED);

                    imshow("Image", temp);

                    int key = waitKey();
                    if (key == 's')
                    {
                        Mat feature(64, 1, CV_32F);
                        for (int k = i*64; k < (i+1)*64; k++)
                        {
                            feature.at<float>(k % 64, 0) = descriptors[k];
                        }

                        featureList.append(feature);
                    }
                }
            }
        }

        features.create(64, featureList.count(), CV_32F);
        features.setTo(Scalar(0));
        for (int i = 0; i < featureList.count(); i++)
        {
            features.col(i) = features.col(i) + featureList.at(i);
        }
    }
    vc.release();
    cvDestroyWindow("Image");
}

void SurfTraining::test(QString videoFile)
{
    QList<Mat> objects;
    objects.append(features);
    sc.setObjects(objects);
    sc.setThresh(thresh);

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
                QList<FoundObject> matches;
                sc.classify(frame, matches);
                for (int i = 0; i < matches.size(); i++)
                {
                    rectangle(frame, Point(matches.at(i).left, matches.at(i).top),
                              Point(matches.at(i).right, matches.at(i).bottom), Scalar(255,0,0), 5);
                }
                imshow("Image", frame);
            }

            int key = waitKey(200);
            if (key == 'q') break;
        }
    }
    vc.release();
    cvDestroyWindow("Image");
}

void SurfTraining::save(QString saveFile)
{
    FileStorage storage(saveFile.toStdString(), CV_STORAGE_WRITE);
    storage.writeObj("Matrix", new CvMat(features));
    storage.release();
}

void SurfTraining::load(QString loadFile)
{
    FileStorage storage(loadFile.toStdString(), CV_STORAGE_READ);
    storage.release();
}


void SurfTraining::setThresh(double thresh)
{
    SurfTraining::thresh = thresh;
}
