#ifndef BALLTRACKER_H
#define BALLTRACKER_H

#include <Framework/objecttracker.h>

class Behaviour_BallFollowing;

class BallTracker : public ObjectTracker
{
public:
    BallTracker(Behaviour_BallFollowing *behave);
    void update(cv::Mat &frame);
};

#endif // BALLTRACKER_H
