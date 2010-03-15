#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "cxcore.h"

class ImageProcessor
{
public:
    ImageProcessor();

    IplImage *threshold(IplImage *img);
    CvMat *features(IplImage *thresh);
};

#endif // IMAGEPROCESSOR_H
