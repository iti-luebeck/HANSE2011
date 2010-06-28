#ifndef SURFCLASSIFIER_H
#define SURFCLASSIFIER_H

#include <QList>
#include <opencv/cxcore.h>
#include <opencv/cvaux.h>
#include <opencv/cv.h>
#include <vector>

using namespace cv;
using namespace std;

typedef struct FoundObject {
    int top;
    int bottom;
    int left;
    int right;
    int numMatches;
    double percentMatches;
} FoundObject;

class SURFClassifier
{
private:
    QList<Mat> objects;
    double thresh;

    void classifyOne(int type, vector<KeyPoint> points, Mat features, FoundObject &match);
public:
    SURFClassifier(double thresh = 0.7, QList<Mat> objects = QList<Mat>());
    void classify(Mat &image, QList<FoundObject> &matches);
    void setObjects(QList<Mat> objects);
    void setThresh(double thresh);
    double getThresh();

};

#endif // SURFCLASSIFIER_H
