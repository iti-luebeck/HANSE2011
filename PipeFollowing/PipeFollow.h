#ifndef PIPEFOLLOW_H
#define PIPEFOLLOW_H


//#include "mainwindow.h"
#include "QString"
#include "cv.h"
//#include "math.h"
#include <cxcore.h>

using namespace cv;

class PipeFollow
{
private:
    int i;
    int threshSegmentation;
    int debug;
    vector<Vec2f> pipe;
    float istWinkel;
    Point schnittP;
    float deltaRohr;
    float kp;
    float speed;
    Point robCenter;
    int pVec;
    Point deltaA;

public:

   PipeFollow(int threshSegmentation = 200, int debug = 0, int deltaRohr = 2);
   void findPipe(QString videoFile);
   void setThresh(int thresh);
   void setDebug(int debug);
   void pRegler();
   void drawLineHough(Mat &frame, double rho, double theta, Scalar color);
   void compIntersect(double rho, double theta);
   void findPipeInPic(QString videoFile);
};



#endif // PIPEFOLLOW_H

