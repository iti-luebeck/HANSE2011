#include "pipetracker.h"
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>

PipeTracker::PipeTracker(Behaviour_PipeFollowing *behave)
{
    this->behave = behave;

    pipeState = PIPE_STATE_NOT_SEEN_YET;

    meanX = 0;
    meanY = 0;
    theta = 0;
    area = 0;

    lastMeanX = 0;
    lastMeanY = 0;
    lastTheta = 0;
    lastArea = 0;
}

void PipeTracker::update(Mat frame)
{
    // Save old values.
    lastMeanX = meanX;
    lastMeanY = meanY;
    lastTheta = theta;
    lastDistanceFromCenter = distanceFromCenter;
    lastArea = area;

    Mat gray = getThresholdChannel(frame);

    // Perform thresholding with automated threshold selection.
    threshold(gray, gray, 0, 255, THRESH_BINARY | THRESH_OTSU);

    // Initialize the moment structure.
    IplImage *ipl = new IplImage(gray);
    CvMoments M;
    cvMoments(ipl, &M, 1);

    // Get number of pipe pixels.
    area = cvGetSpatialMoment(&M, 0, 0);

    // The pipe is seen if:
    //      1. at least 1/20th of the image is "pipe"
    //      2. at most half of the image is "pipe"
    // We have successfully passed the pipe if
    //      1. we see the pipe
    //      2. the pipe angle is about 0
    //      3. the pipe center is in the lower quater of the image.
    // If there is a small part of the pipe seen, we set states to lost at that particular direction.
    if (area < 0.5 * gray.cols * gray.rows) {
        // First order moments -> mean position
        double m10 = cvGetSpatialMoment( &M, 1, 0 ) / area;
        double m01 = cvGetSpatialMoment( &M, 0, 1 ) / area;

        meanX = m10;
        meanY = m01;

        if (area > 0.05 * gray.cols * gray.rows) {
            pipeState = PIPE_STATE_IS_SEEN;

            // Second order moments -> orientation
            double mu11 = cvGetCentralMoment( &M, 1, 1 ) / area;
            double mu20 = cvGetCentralMoment( &M, 2, 0 ) / area;
            double mu02 = cvGetCentralMoment( &M, 0, 2 ) / area;
            theta = 0.5 * atan2( 2 * mu11 , ( mu20 - mu02 ) );

            // Theta is now the rotation angle reletive to the x axis.
            // We want it relative to the y axis -> 90° ccw
            theta -= CV_PI / 2;
            if (theta < -CV_PI) {
                theta += CV_PI;
            }

            if (theta < -CV_PI/2) {
                theta += CV_PI;
            } else if (theta > CV_PI/2) {
                theta -= CV_PI;
            }

            if (abs(theta) < CV_PI / 6.0 && meanY > 0.75 * gray.rows &&
                    meanX > 0.2 * gray.cols && meanX < 0.8 * gray.cols) {
                pipeState = PIPE_STATE_PASSED;
            }

            distanceFromCenter = computeIntersection(meanX, meanY, theta);
        } else {
            if (meanX < 0.25 * gray.cols) {
                pipeState = PIPE_STATE_LOST_LEFT;
            } else if (meanX > 0.75 * gray.cols) {
                pipeState = PIPE_STATE_LOST_RIGHT;
            } else if (meanY < 0.25 * gray.rows) {
                pipeState = PIPE_STATE_LOST_BOTTOM;
            } else if (meanY > 0.75 * gray.rows) {
                pipeState = PIPE_STATE_LOST_TOP;
            } else {
                if (pipeState == PIPE_STATE_IS_SEEN) {
                    pipeState = PIPE_STATE_LOST;
                }
            }
        }
    } else {
        if (pipeState == PIPE_STATE_IS_SEEN) {
            if (lastMeanX < 0.25 * gray.cols) {
                pipeState = PIPE_STATE_LOST_LEFT;
            } else if (lastMeanX > 0.75 * gray.cols) {
                pipeState = PIPE_STATE_LOST_RIGHT;
            } else if (lastMeanY < 0.25 * gray.rows) {
                pipeState = PIPE_STATE_LOST_TOP;
            } else if (lastMeanY > 0.75 * gray.rows) {
                pipeState = PIPE_STATE_LOST_BOTTOM;
            } else {
                pipeState = PIPE_STATE_LOST;
            }
        }
    }
}

Mat PipeTracker::getThresholdChannel(Mat frame)
{
    int colorSpace =  behave->getSettingsValue("convColor", 0).toInt();
    vector<Mat> colors;
    Mat gray(frame.rows, frame.cols, CV_8UC1);

    if (colorSpace == 0 || colorSpace >= 4) {
        split(frame, colors);
        colors[2].copyTo(gray);
    } else {
        cvtColor(frame, frame, CV_RGB2HSV);
        split(frame, colors);
        colors[colorSpace - 1].copyTo(gray);
    }
    return gray;
}

double PipeTracker::computeIntersection(double meanX, double meanY, double theta)
{
    double robX = behave->getSettingsValue("robCenterX", 320).toDouble();
    double robY = behave->getSettingsValue("robCenterY", 240).toDouble();

    double nzero[2] = {cos(theta), sin(theta)};
    double d = meanX * nzero[0] + meanY * nzero[1];

    // nzero * p - d
    return (nzero[0] * robX) + (nzero[1] * robY) - d;
}

double PipeTracker::getPipeAngle()
{
    return theta;
}

double PipeTracker::getPipeDistance()
{
    return distanceFromCenter;
}

double PipeTracker::getMeanX()
{
    return meanX;
}

double PipeTracker::getMeanY()
{
    return meanY;
}

QString PipeTracker::getPipeState()
{
    return pipeState;
}
