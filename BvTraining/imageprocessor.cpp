#include "imageprocessor.h"
#include <cv.h>
#include <highgui.h>
#include <blob.h>
#include <BlobResult.h>

ImageProcessor::ImageProcessor()
{
}

void ImageProcessor::my_convert(Mat rgb, Mat &gray)
{

    for (int i = 0; i < rgb.rows; i++)
    {
        for (int j = 0; j < rgb.cols; j++)
        {
            Vec<unsigned char, 3> v = rgb.at<Vec<unsigned char, 3> >(i, j);
            gray.at<unsigned char>(i, j) = 0.299 * v[0] + 0.587 * v[1] + 0.114 * v[2];
        }
    }
}

void ImageProcessor::threshold(Mat img, Mat &thresh)
{
    // medianBlur(img, img, 3);

    Mat imgGray(img.size(), CV_8UC1, Scalar(0));
    my_convert(img, imgGray);

    thresh.create(imgGray.size(), CV_8UC1);
    thresh.setTo(Scalar(0));
    adaptiveThreshold(imgGray, thresh, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 31, 21);
}

void ImageProcessor::features(Mat &thresh, Mat &features, Mat &classes)
{
    IplImage *iplThresh = new IplImage(thresh);
    IplImage *disp = cvCreateImage(cvSize(iplThresh->width, iplThresh->height), IPL_DEPTH_8U, 3);
    cvSet(disp, cvScalar(255,255,255));

    CBlobResult blobs(iplThresh, NULL, 255);
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 50);

    for (int i = 0; i < blobs.GetNumBlobs(); i++)
    {
        CBlob *b = blobs.GetBlob(i);
        b->FillBlob(disp, cvScalar(0,0,0));
    }

    features.create(blobs.GetNumBlobs(), 2, CV_32FC1);
    classes.create(blobs.GetNumBlobs(), 1, CV_32FC1);
    for (int i = 0; i < blobs.GetNumBlobs(); i++)
    {
        CBlob *b = blobs.GetBlob(i);

        features.at<float>(i,0) = b->Area();
        features.at<float>(i,1) = b->StdDev(iplThresh);

        b->FillBlob(disp, cvScalar(255,0,0));

        imshow("Blob", Mat(disp));

        int key = waitKey();
        classes.at<float>(i,0) = (float)(key - '0');

        b->FillBlob(disp, cvScalar(0,0,0));
    }
}

void ImageProcessor::features(Mat &thresh, Mat &features, SVMClassifier &svm)
{
    IplImage *iplThresh = new IplImage(thresh);
    IplImage *disp = cvCreateImage(cvSize(iplThresh->width, iplThresh->height), IPL_DEPTH_8U, 3);
    cvSet(disp, cvScalar(255,255,255));

    CBlobResult blobs(iplThresh, NULL, 255);
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 50);

    if (blobs.GetNumBlobs() > 0)
    {
        features.create(blobs.GetNumBlobs(), 2, CV_32FC1);
        for (int i = 0; i < blobs.GetNumBlobs(); i++)
        {
            CBlob *b = blobs.GetBlob(i);

            features.at<float>(i,0) = b->Area();
            features.at<float>(i,1) = b->StdDev(iplThresh);
        }

        CvMat *f = new CvMat(features);
        CvMat *c = svm.classify(f);

        for (int i = 0; i < blobs.GetNumBlobs(); i++)
        {
            CBlob *b = blobs.GetBlob(i);
            if (cvmGet(c, 0, i) == 1)
            {
                b->FillBlob(disp, cvScalar(0,0,255));
            }
            else
            {
                b->FillBlob(disp, cvScalar(0,0,0));
            }
        }
    }

    cvShowImage("Klassifikation", disp);
}
