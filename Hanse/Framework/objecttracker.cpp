#include "objecttracker.h"
#include <opencv/cv.h>
#include <Behaviour_BallFollowing/blobs/blob.h>
#include <Behaviour_BallFollowing/blobs/BlobResult.h>

using namespace cv;

ObjectTracker::ObjectTracker(int channel, bool automatic, int thres, bool inverted)
{
    this->channel = channel;
    this->automatic = automatic;
    this->thres = thres;
    this->inverted = inverted;
    init();
}

void ObjectTracker::init()
{
    reset();
}

void ObjectTracker::reset()
{
    state = STATE_NOT_SEEN_YET;

    area = 0;
    meanX = 0;
    meanY = 0;
    orientation = 0;

    lastArea = 0;
    lastMeanX = 0;
    lastMeanY = 0;
    lastOrientation = 0;
}

void ObjectTracker::update(cv::Mat)
{
    lastArea = area;
    lastMeanX = meanX;
    lastMeanY = meanY;
    lastOrientation = orientation;
}

cv::Mat ObjectTracker::getThresholdChannel(cv::Mat frame)
{
    vector<Mat> colors;
    Mat gray(frame.rows, frame.cols, CV_8UC1);

    if (channel >= 3) {
        cvtColor(frame, frame, CV_RGB2HSV);
    }
    split(frame, colors);
    colors[channel % 3].copyTo(gray);
    return gray;
}

double ObjectTracker::applyThreshold(cv::Mat &gray)
{
    if (inverted) {
        if (automatic) {
            return threshold(gray, gray, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
        } else {
            return threshold(gray, gray, thres, 255, THRESH_BINARY_INV);
        }
    } else {
        if (automatic) {
            return threshold(gray, gray, 0, 255, THRESH_BINARY | THRESH_OTSU);
        } else {
            return threshold(gray, gray, thres, 255, THRESH_BINARY);
        }
    }
}

void ObjectTracker::extractLargestBlob(cv::Mat &gray)
{
    IplImage *thresh = new IplImage(gray);

    CBlobResult blobs(thresh, NULL, 0);
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, 500 );

    // Get largest blob.
    int maxArea = 0;
    int maxBlob = -1;
    for (int j = 0; j < blobs.GetNumBlobs(); j++) {
        CBlob *blob = blobs.GetBlob(j);
        if (blob->Area() > maxArea) {
            maxArea = blob->Area();
            maxBlob = j;
        }
    }

    if (maxBlob >= 0) {
        area = blobs.GetBlob(maxBlob)->Moment(0, 0);

        // First order moments -> mean position
        double m10 = blobs.GetBlob(maxBlob)->Moment(1, 0) / area;
        double m01 = blobs.GetBlob(maxBlob)->Moment(0, 1) / area;
        meanX = m10;
        meanY = m01;

        // Second order moments -> orientation
        double mu11 = blobs.GetBlob(maxBlob)->Moment(1, 1) / area;
        double mu20 = blobs.GetBlob(maxBlob)->Moment(2, 0) / area;
        double mu02 = blobs.GetBlob(maxBlob)->Moment(0, 2) / area;
        orientation = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );

        // Draw blob to gray image.
        gray = gray.setTo(Scalar(0));
        blobs.GetBlob(maxBlob)->FillBlob(thresh, cvScalar(255), 0, 0);
    } else {

    }
}

void ObjectTracker::estimateMoments(cv::Mat &gray)
{
    IplImage *ipl = new IplImage(gray);
    CvMoments M;
    cvMoments(ipl, &M, 1);

    // Get number of pipe pixels.
    area = cvGetSpatialMoment(&M, 0, 0);

    // First order moments -> mean position
    double m10 = cvGetSpatialMoment( &M, 1, 0 ) / area;
    double m01 = cvGetSpatialMoment( &M, 0, 1 ) / area;
    meanX = m10;
    meanY = m01;

    // Second order moments -> orientation
    double mu11 = cvGetCentralMoment( &M, 1, 1 ) / area;
    double mu20 = cvGetCentralMoment( &M, 2, 0 ) / area;
    double mu02 = cvGetCentralMoment( &M, 0, 2 ) / area;
    orientation = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );

    // Theta is now the rotation angle reletive to the x axis.
    // We want it relative to the y axis -> 90° ccw
    orientation -= CV_PI / 2;
    if (orientation < -CV_PI) {
        orientation += CV_PI;
    }

    if (orientation < -CV_PI/2) {
        orientation += CV_PI;
    } else if (orientation > CV_PI/2) {
        orientation -= CV_PI;
    }

}

double ObjectTracker::getArea()
{
    return area;
}

double ObjectTracker::getMeanX()
{
    return meanX;
}

double ObjectTracker::getMeanY()
{
    return meanY;
}

double ObjectTracker::getOrientation()
{
    return orientation;
}

QString ObjectTracker::getState()
{
    return state;
}
