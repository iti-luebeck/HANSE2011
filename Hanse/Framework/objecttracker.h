#ifndef OBJECTTRACKER_H
#define OBJECTTRACKER_H

#include <QtCore>
#include <opencv/cxcore.h>

#define STATE_NOT_SEEN_YET     "not seen"
#define STATE_IS_SEEN          "is seen"
#define STATE_PASSED           "was passed"
#define STATE_LOST             "lost"
#define STATE_LOST_LEFT        "lost left"
#define STATE_LOST_RIGHT       "lost right"
#define STATE_LOST_BOTTOM      "lost bottom"
#define STATE_LOST_TOP         "lost top"

class RobotModule;

class ObjectTracker : public QObject
{
    Q_OBJECT

public:
    ObjectTracker(RobotModule *parent);
    void init();
    void reset();
    void update(cv::Mat &frame);

    QString getColorSpace();
    void setColorSpace(QString colorSpace);
    int getChannel();
    void setChannel(int channel);
    bool isAutomaticThreshold();
    void setAutomaticThreshold(bool automatic);
    double getThreshold();
    void setThreshold(double thres);
    bool isInverted();
    void setInverted(bool inverted);

    void grabFrame(cv::Mat &frame);
    void grabGray(cv::Mat &gray);
    void grabBinary(cv::Mat &binary);

    double getArea();
    double getMeanX();
    double getMeanY();
    double getOrientation();
    QString getState();

protected:
    void getThresholdChannel();
    double applyThreshold();
    void extractLargestBlob();
    void estimateMoments();
    void updateSettings();
    void loadFromSettings();

protected:
    RobotModule *parent;

    cv::Mat frame;
    cv::Mat gray;
    cv::Mat binary;

    QString colorSpace;
    int channel;

    bool automatic;
    double thres;
    bool inverted;
    QString state;

    double area;
    double meanX;
    double meanY;
    double orientation;

    double lastArea;
    double lastMeanX;
    double lastMeanY;
    double lastOrientation;

signals:
    void updateComplete();
};

#endif // OBJECTTRACKER_H
