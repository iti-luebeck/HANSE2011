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
class Module_Simulation;

class SonarEchoFilter: public QObject
{
    Q_OBJECT

public:
    SonarEchoFilter(Module_SonarLocalization* sonar, Module_XsensMTi *mti, Module_Simulation *sim);

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
    void gradientFilter(SonarEchoData &data);

    cv::Mat byteArray2Mat(QByteArray array);
    QVector<double> mat2QVector(cv::Mat& mat);
    QByteArray mat2byteArray(cv::Mat& mat);
    QList<float> mat2List(cv::Mat& mat);

    void getNoNoiseFilter(QVector<int> &vec);
    float getLastObservationHeading();

signals:
    void newImage(QList<QVector2D> observations);
    void newSonarEchoData(QList<SonarEchoData> data);
    void newSonarPlotData(QList<SonarEchoData> data);

private slots:
    void newSonarData(SonarReturnData data);

private:
    // max value of any samples coming from the sonar
    const static float MAX = 127;

    QList<SonarEchoData> candidates;
    //grouping
    int groupID;
    int newDirection;
    double temp_area;

    int currentID;
    double swipedArea;
    int darknessCount;
    float lastMaxValue;
    QList<float> lastMaxValues;
    QList<int> ks;

    void addToList(QList<QVector2D>& list, const QVector2D p);

    bool DEBUG;
//    QSettings& s;

    Module_SonarLocalization* sloc;
    Module_XsensMTi *mti;
    Module_Simulation *sim;

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

    float lastValidDataHeading;
    float currentDataHeading;

};

#endif // SONARECHOFILTER_H
