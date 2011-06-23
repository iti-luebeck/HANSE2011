#include "pipetracker.h"
#include <Behaviour_PipeFollowing/behaviour_pipefollowing.h>

PipeTracker::PipeTracker(Behaviour_PipeFollowing *behave) : ObjectTracker(CHANNEL_B)
{
    this->behave = behave;
    reset();
}

void PipeTracker::init()
{
    reset();
}

void PipeTracker::reset()
{
    ObjectTracker::reset();
    distanceFromCenter = 0;
    lastDistanceFromCenter = 0;
}

void PipeTracker::update(Mat frame)
{
    ObjectTracker::update(frame);
    lastDistanceFromCenter = distanceFromCenter;

    Mat gray = getThresholdChannel(frame);
    applyThreshold(gray);
    estimateMoments(gray);

    // The pipe is seen if:
    //      1. at least 1/20th of the image is "pipe"
    //      2. at most half of the image is "pipe"
    // We have successfully passed the pipe if
    //      1. we see the pipe
    //      2. the pipe angle is about 0
    //      3. the pipe center is in the lower quater of the image.
    // If there is a small part of the pipe seen, we set states to lost at that particular direction.
    if (area < 0.5 * gray.cols * gray.rows) {

        if (area > 0.05 * gray.cols * gray.rows) {
            state = STATE_IS_SEEN;

            if (abs(orientation) < CV_PI / 6.0 && meanY > 0.75 * gray.rows &&
                    meanX > 0.2 * gray.cols && meanX < 0.8 * gray.cols) {
                state = STATE_PASSED;
            }

            distanceFromCenter = computeIntersection(meanX, meanY, orientation);
        } else {
            if (state == STATE_IS_SEEN) {
                if (meanX < 0.25 * gray.cols) {
                    state = STATE_LOST_LEFT;
                } else if (meanX > 0.75 * gray.cols) {
                    state = STATE_LOST_RIGHT;
                } else if (meanY < 0.25 * gray.rows) {
                    state = STATE_LOST_TOP;
                } else if (meanY > 0.75 * gray.rows) {
                    state = STATE_LOST_BOTTOM;
                } else {
                    state = STATE_LOST;
                }
            }
        }
    } else {
        if (state == STATE_IS_SEEN) {
            if (lastMeanX < 0.25 * gray.cols) {
                state = STATE_LOST_LEFT;
            } else if (lastMeanX > 0.75 * gray.cols) {
                state = STATE_LOST_RIGHT;
            } else if (lastMeanY < 0.25 * gray.rows) {
                state = STATE_LOST_TOP;
            } else if (lastMeanY > 0.75 * gray.rows) {
                state = STATE_LOST_BOTTOM;
            } else {
                state = STATE_LOST;
            }
        }
    }
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

double PipeTracker::getDistanceToCenter()
{
    return distanceFromCenter;
}
