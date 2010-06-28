#include "surftraining.h"

#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include "helpers.h"
#include <iostream>
#include <vector>
#include <QDir>

using namespace cv;
using namespace std;

SurfTraining::SurfTraining()
{
}

void SurfTraining::train(QList<int> frameList, QString videoFile, bool isDir)
{
    QList<Mat> featureList;
    SURF surf(1500.0);

    if ( isDir )
    {
        QDir dir( videoFile );
        dir.setFilter( QDir::Files );
        QStringList filters;
        filters << "*.jpg";
        dir.setNameFilters( filters );
        QStringList files = dir.entryList();
        Mat frame;
        Mat frameGray;

        namedWindow("Image", 0);
        for ( int k = 0; k < frameList.count(); k++ )
        {
            QString filePath = videoFile;
            filePath.append( "/" );
            filePath.append( files[k] );

            frame = imread( filePath.toStdString() );

            if ( !frame.empty() )
            {
                vector<KeyPoint> keyPoints;
                vector<float> descriptors;

                Helpers::convertRGB2Gray(frame, frameGray);

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

            features.create(64, featureList.count(), CV_32F);
            features.setTo(Scalar(0));
            for (int i = 0; i < featureList.count(); i++)
            {
                features.col(i) = features.col(i) + featureList.at(i);
            }
        }
        cvDestroyWindow("Image");
    }
    else
    {
        VideoCapture vc(videoFile.toStdString());
        if (vc.isOpened())
        {
            namedWindow("Image", 0);
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

                    Helpers::convertRGB2Gray(frame, frameGray);

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
}

void SurfTraining::test(QString videoFile, bool isDir)
{
    if ( isDir )
    {
        QList<Mat> objects;
        objects.append(features);
        sc.setObjects(objects);
        sc.setThresh(thresh);
        
        QDir dir( videoFile );
        dir.setFilter( QDir::Files );
        QStringList filters;
        filters << "*.jpg";
        dir.setNameFilters( filters );
        QStringList files = dir.entryList();
        Mat frame;

        namedWindow("Image", 0);
        for ( int i = 0; i < files.count(); i++ )
        {
            QString filePath = videoFile;
            filePath.append( "/" );
            filePath.append( files[i] );

            frame = imread( filePath.toStdString() );
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
            
            int key = waitKey(500);
            if (key == 'q') break;
        }
        cvDestroyWindow("Image");
    }
    else
    {
        QList<Mat> objects;
        objects.append(features);
        sc.setObjects(objects);
        sc.setThresh(thresh);
    
        VideoCapture vc(videoFile.toStdString());
        if (vc.isOpened())
        {
            namedWindow("Image", 0);
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
    
                int key = waitKey(500);
                if (key == 'q') break;
            }
        }
        vc.release();
        cvDestroyWindow("Image");
    }
}

void SurfTraining::liveTest(int webcamID)
{
    QList<Mat> objects;
    objects.append(features);
    sc.setObjects(objects);
    sc.setThresh(thresh);
    int       key = 0;
    VideoCapture capture = VideoCapture(webcamID);
    if ( !capture.grab() ) {
        fprintf( stderr, "Cannot open initialize webcam!\n" );
    }
    cvNamedWindow( "Image", CV_WINDOW_AUTOSIZE );
    while( key != 'q' ) {
        cv::Mat frame;
        capture.retrieve(frame,0);
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
        key = cvWaitKey( 1 );
    }
    cvDestroyWindow("Image");
    capture.release();
}

void SurfTraining::save(QString saveFile)
{
    FileStorage storage(saveFile.toStdString(), CV_STORAGE_WRITE);
    storage.writeObj("Matrix", new CvMat(features));
    storage.release();
}

void SurfTraining::load(QString loadFile)
{
    CvMat* featureMat = (CvMat*)cvLoad(loadFile.toAscii().data());
    features = Mat(featureMat,true);
}


void SurfTraining::setThresh(double thresh)
{
    SurfTraining::thresh = thresh;
}
