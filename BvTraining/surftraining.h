#ifndef SURFTRAINING_H
#define SURFTRAINING_H

#include <QString>
#include <QList>
#include <cxcore.h>
#include "surfclassifier.h"

using namespace cv;

class SurfTraining
{
private:
    Mat features;
    SURFClassifier sc;
    double thresh;

public:
    SurfTraining();

    void train(QList<int> frameList, QString videoFile);
    void test(QString videoFile);
    void save(QString saveFile);
    void load(QString loadFile);

    void setThresh(double thresh);
};

#endif // SURFTRAINING_H
