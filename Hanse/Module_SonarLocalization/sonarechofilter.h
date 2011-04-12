#ifndef SONARECHOFILTER_H
#define SONARECHOFILTER_H

#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>
#include <log4qt/logger.h>
#include <Module_SonarLocalization/sonarechodata.h>
#include "Module_SLTraining/SVMClassifier.h"

class Module_SonarLocalization;
class Module_XsensMTi;

class SonarEchoFilter: public QObject
{
    Q_OBJECT

public:
    SonarEchoFilter(Module_SonarLocalization* sonar, Module_XsensMTi *mti);

    const static int N = 250;

    void reset();

    /* DEBUG information; for View's eyes only */
    QMap<QDateTime,QVector<double> > rawHistory;
    QMap<QDateTime,QVector<double> > filteredHistory;
    QMap<QDateTime, int > kHistory;
    QMap<QDateTime, QVector<double> > threshHistory;
    QMap<QDateTime, QVector<double> > varHistory;
    QMap<QDateTime, QVector<double> > meanHistory;

    //filter chain
    void filterEcho(SonarEchoData &data);
    void medianFilter(SonarEchoData &data);
    void findWall(SonarEchoData &data);
    void gradientFilter(SonarEchoData &data);
    void extractFeatures(SonarEchoData &data);

    cv::Mat byteArray2Mat(QByteArray array);
    QVector<double> mat2QVector(cv::Mat& mat);
    QByteArray mat2byteArray(cv::Mat& mat);
    QList<float> mat2List(cv::Mat& mat);

    void getNoNoiseFilter(QVector<int> &vec);

signals:
    void newImage(QList<QVector2D> observations);
    void newSonarEchoData(QList<SonarEchoData> data);

private slots:
    void newSonarData(SonarReturnData data);

private:
    // max value of any samples coming from the sonar
    const static float MAX = 127;

    SVMClassifier* svm;

    QList<SonarEchoData> candidates;
    //grouping
    int groupID;
    int diff;
    int newDirection;
    int temp_area;

    QList<int> localKlist;
    QList<double> localKlistHeading;
    QList<int> localKlistID;
    int currentID;
    double swipedArea;
    int darknessCount;
    float lastMaxValue;
    QList<float> lastMaxValues;

    void addToList(QList<QVector2D>& list, const QVector2D p);

    bool DEBUG;
//    QSettings& s;

    Module_SonarLocalization* sloc;
    Module_XsensMTi *mti;

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

    //filter chain
    void applyHeuristic();
    void grouping();
    void sendImage();
    //other
    cv::Mat noiseMat;
    void initNoiseMat();
    float prevWallCandidate;

};

#endif // SONARECHOFILTER_H
