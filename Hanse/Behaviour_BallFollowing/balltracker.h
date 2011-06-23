#ifndef BALLTRACKER_H
#define BALLTRACKER_H

#include <Framework/objecttracker.h>

class BallTracker : public ObjectTracker
{
public:
    BallTracker();
    void update(cv::Mat frame);
    cv::Mat getGray();

private:
    cv::Mat gray;
};

#endif // BALLTRACKER_H
