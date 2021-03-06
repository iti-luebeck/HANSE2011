#include "balltracker.h"
#include <Behaviour_BallFollowing/behaviour_ballfollowing.h>

using namespace cv;

BallTracker::BallTracker(Behaviour_BallFollowing *behave) : ObjectTracker(behave)
{
}

void BallTracker::update(cv::Mat &frame)
{
    ObjectTracker::update(frame);

//    applyHomomorphic();
//    applyClahe();
    findMovingObject();

    applyThreshold();
    extractLargestBlob();

    // The midwater target is seen if:
    //      1. at least 500 pixels of the image are "ball"
    //      2. at most half of the image is "ball"
    // We have successfully passed the ball if
    //      1. we do not see the ball
    //      2. the ball was last seen in the top or bottom center of the image

    if (area > 300 && area < 0.3 * gray.cols * gray.rows) {
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

    if (state == STATE_IS_SEEN) {
        ellipse(this->frame, RotatedRect(Point(meanX, meanY), Size(std::sqrt(area), std::sqrt(area)), 0.0f), Scalar(255,0,0), 5);
    }

    emit updateComplete();
}
