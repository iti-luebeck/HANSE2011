#include "surftraining.h"

#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include "helpers.h"
#include "surf/surflib.h"
#include <iostream>

using namespace cv;

SurfTraining::SurfTraining()
{
}

void SurfTraining::train(QList<int> frameList, QString videoFile)
{
    QList<Mat> featureList;
    SURF surf(2.0);

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

                IplImage *iplFrame = new IplImage(frame);
                IplImage *bla = cvCreateImage(cvSize(iplFrame->width, iplFrame->height), IPL_DEPTH_8U, 1);
                cvCvtColor(iplFrame, bla, CV_BGR2GRAY);
                cvShowImage("Image", bla);

                Helpers::convertBGR2Gray(frame, frameGray);

                surf(frameGray, Mat::ones(frameGray.size(), CV_8UC1), keyPoints, descriptors);

                for (int i = 0; i < keyPoints.size(); i++)
                {
                    circle(frame, Point(keyPoints[i].pt.x, keyPoints[i].pt.y), 10, Scalar(0,0,255), 1, CV_FILLED);
                }
                imshow("Image", frame);

                //featureList.append(features);
                //totalFeatures += features.rows;
            }
        }
    }
    vc.release();
    cvDestroyWindow("Image");
}

void SurfTraining::test(QString videoFile)
{
}

void SurfTraining::save(QString saveFile)
{
}

void SurfTraining::load(QString loadFile)
{
}
