#ifndef OBJECTTRACKER_H
#define OBJECTTRACKER_H

#include <QtCore>
#include <opencv/cxcore.h>

#define CHANNEL_R   0
#define CHANNEL_G   1
#define CHANNEL_B   2
#define CHANNEL_H   3
#define CHANNEL_S   4
#define CHANNEL_V   5

#define STATE_NOT_SEEN_YET     "not seen"
#define STATE_IS_SEEN          "is seen"
#define STATE_PASSED           "was passed"
#define STATE_LOST             "lost"
#define STATE_LOST_LEFT        "lost left"
#define STATE_LOST_RIGHT       "lost right"
#define STATE_LOST_BOTTOM      "lost bottom"
#define STATE_LOST_TOP         "lost top"

class ObjectTracker
{
public:
    ObjectTracker(int channel);
    void init();
    void reset();
    void update(cv::Mat frame);

    double getArea();
    double getMeanX();
    double getMeanY();
    double getOrientation();
    QString getState();

protected:
    cv::Mat getThresholdChannel(cv::Mat frame);
    void applyThreshold(cv::Mat &gray);
    void extractLargestBlob(cv::Mat &gray);
    void estimateMoments(cv::Mat &gray);

protected:
    int channel;
    QString state;

    double area;
    double meanX;
    double meanY;
    double orientation;

    double lastArea;
    double lastMeanX;
    double lastMeanY;
    double lastOrientation;
};

#endif // OBJECTTRACKER_H
