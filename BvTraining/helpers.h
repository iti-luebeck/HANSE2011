#ifndef HELPERS_H
#define HELPERS_H

#include <cxcore.h>

using namespace cv;

class Helpers
{
public:
    Helpers();
    static void convertRGB2Gray(Mat rgb, Mat &gray);
    static void convertBGR2Gray(Mat bgr, Mat &gray);
};

#endif // HELPERS_H
