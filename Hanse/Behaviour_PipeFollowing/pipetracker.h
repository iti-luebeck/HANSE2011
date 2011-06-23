#ifndef PIPETRACKER_H
#define PIPETRACKER_H

#include <Framework/objecttracker.h>

class Behaviour_PipeFollowing;

class PipeTracker : public ObjectTracker
{
public:
    PipeTracker(Behaviour_PipeFollowing *behave);
    void init();
    void reset();
    void update(cv::Mat frame);
    double getDistanceToCenter();

private:
    double computeIntersection(double meanX, double meanY, double theta);

private:
    Behaviour_PipeFollowing *behave;
    double distanceFromCenter;
    double lastDistanceFromCenter;
};

#endif // PIPETRACKER_H
