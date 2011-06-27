#include "balltracker.h"

using namespace cv;

BallTracker::BallTracker() : ObjectTracker(CHANNEL_S)
{
}

void BallTracker::update(cv::Mat frame)
{
    ObjectTracker::update(frame);

    gray = getThresholdChannel(frame);
    double T = applyThreshold(gray);
    extractLargestBlob(gray);

    // The midwater target is seen if:
    //      1. at least 500 pixels of the image are "ball"
    //      2. at most half of the image is "ball"
    // We have successfully passed the ball if
    //      1. we do not see the ball
    //      2. the ball was last seen in the top or bottom center of the image

    if (area > 500 && area < 0.5 * gray.cols * gray.rows && T > 50) {
        state = STATE_IS_SEEN;
    } else {
        if (state == STATE_IS_SEEN) {
            if (lastMeanX > 0.4 * gray.cols && lastMeanX < 0.6 * gray.cols) {
                state = STATE_PASSED;
            } else {
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
}

Mat BallTracker::getGray()
{
    return gray;
}
