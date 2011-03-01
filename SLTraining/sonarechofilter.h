#ifndef SONARECHOFILTER_H
#define SONARECHOFILTER_H

#include <sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>

class SonarEchoFilter
{

public:
    SonarEchoFilter();

    const static int N = 250;

    void reset();

    /* DEBUG information; for View's eyes only */
    QMap<QDateTime,QVector<double> > rawHistory;
    QMap<QDateTime,QVector<double> > filteredHistory;
    QMap<QDateTime, int > kHistory;
    QMap<QDateTime, QVector<double> > threshHistory;
    QMap<QDateTime, QVector<double> > varHistory;
    QMap<QDateTime, QVector<double> > meanHistory;

    QByteArray newSonarData(SonarReturnData data);
    int findWall(SonarReturnData data,const QByteArray echoBA);


//signals:
//    void newImage(QList<QVector2D> observations);

private:
    // max value of any samples coming from the sonar
    const static float MAX = 127;

    QList<int> localKlist;
    QList<double> localKlistHeading;
    QList<int> localKlistID;
    int currentID;
    double swipedArea;
    int darknessCount;

    void addToList(QList<QVector2D>& list, const QVector2D p);

    bool DEBUG;
//    QSettings& s;

    cv::Mat filterEcho(SonarReturnData data,const cv::Mat& echo);
    QByteArray mat2byteArray(cv::Mat& mat);
    QVector<double> mat2QVector(cv::Mat& mat);
    cv::Mat byteArray2Mat(QByteArray array);

    //ehemals settings
    float gGaussFactor;
    float gVarianceTH;
    int gWallWindowSize;
    float gLargePeakTH;
    float gMeanBehindTH;

    cv::Mat noiseMat;
    void initNoiseMat();

};

#endif // SONARECHOFILTER_H
