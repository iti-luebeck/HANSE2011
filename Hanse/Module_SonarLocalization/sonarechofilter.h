#ifndef SONARECHOFILTER_H
#define SONARECHOFILTER_H

#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>
#include <log4qt/logger.h>
#include <Module_SonarLocalization/sonarechodata.h>
#include "SVMClassifier.h"

class Module_SonarLocalization;

class SonarEchoFilter: public QObject
{
    Q_OBJECT

public:
    SonarEchoFilter(Module_SonarLocalization* sonar);

    const static int N = 250;

    void reset();

    /* DEBUG information; for View's eyes only */
    QMap<QDateTime,QVector<double> > rawHistory;
    QMap<QDateTime,QVector<double> > filteredHistory;
    QMap<QDateTime, int > kHistory;
    QMap<QDateTime, QVector<double> > threshHistory;
    QMap<QDateTime, QVector<double> > varHistory;
    QMap<QDateTime, QVector<double> > meanHistory;



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

    void addToList(QList<QVector2D>& list, const QVector2D p);

    bool DEBUG;
//    QSettings& s;

    Module_SonarLocalization* sloc;

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

    //filter chain
    void filterEcho(SonarEchoData &data);
    void gaussFilter(SonarEchoData &data);
    void findWall(SonarEchoData &data);
    void extractFeatures(SonarEchoData &data);
    void applyHeuristic();
    void grouping();
    void sendImage();
    //other
    void getNoNoiseFilter(QVector<int> &vec);
    cv::Mat noiseMat;
    void initNoiseMat();
    float prevWallCandidate;
    //helpers
    cv::Mat byteArray2Mat(QByteArray array);
    QVector<double> mat2QVector(cv::Mat& mat);
    QByteArray mat2byteArray(cv::Mat& mat);

};

#endif // SONARECHOFILTER_H
