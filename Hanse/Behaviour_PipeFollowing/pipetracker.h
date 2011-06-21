#ifndef PIPETRACKER_H
#define PIPETRACKER_H

#include <QtCore>
#include <opencv/cxcore.h>

using namespace cv;

class Behaviour_PipeFollowing;

#define PIPE_STATE_NOT_SEEN_YET     "pipe not seen"
#define PIPE_STATE_IS_SEEN          "pipe is seen"
#define PIPE_STATE_PASSED           "pipe was passed"
#define PIPE_STATE_LOST             "pipe lost"
#define PIPE_STATE_LOST_LEFT        "pipe lost left"
#define PIPE_STATE_LOST_RIGHT       "pipe lost right"
#define PIPE_STATE_LOST_BOTTOM      "pipe lost bottom"
#define PIPE_STATE_LOST_TOP         "pipe lost top"

class PipeTracker
{
public:
    PipeTracker(Behaviour_PipeFollowing *behave);
    void reset();
    void update(Mat frame);
    double getPipeAngle();
    double getPipeDistance();
    double getMeanX();
    double getMeanY();
    QString getPipeState();

private:
    Mat getThresholdChannel(Mat frame);
    double computeIntersection(double meanX, double meanY, double theta);

    Behaviour_PipeFollowing *behave;

    QString pipeState;

    double meanX;
    double meanY;
    double theta;
    double distanceFromCenter;
    double area;

    double lastMeanX;
    double lastMeanY;
    double lastTheta;
    double lastDistanceFromCenter;
    double lastArea;
};

#endif // PIPETRACKER_H
