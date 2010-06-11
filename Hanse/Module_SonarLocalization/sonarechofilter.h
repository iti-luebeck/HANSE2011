#ifndef SONARECHOFILTER_H
#define SONARECHOFILTER_H

#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>
#include <log4qt/logger.h>

class Module_ScanningSonar;

class SonarEchoFilter: public QObject
{
    Q_OBJECT

public:
    SonarEchoFilter(Module_ScanningSonar* sonar);

    const static int N = 250;

    const static float stdDevInWindowTH = 0.04;
    const static float meanBehindTH = 1;

    QMap<QDateTime,QVector<double> > rawHistory;
    QMap<QDateTime,QVector<double> > filteredHistory;
    QMap<QDateTime, int > kHistory;
    QMap<QDateTime, QVector<double> > threshHistory;
    QMap<QDateTime, QVector<double> > varHistory;
    QMap<QDateTime, QVector<double> > meanHistory;
    QVector<int> K_history;
    QVector<int> localKlist;
    QVector<double> localKlistHeading;
    QVector<int> localKlistID;
    int currentID;
    QVector<QVector2D> posArray;

    double swipedArea;

    int darknessCount;

    Module_ScanningSonar* sonar;

    cv::Mat filterEcho(SonarReturnData data,const cv::Mat& echo);

    int findWall(SonarReturnData data,const cv::Mat& echo);

    cv::Mat byteArray2Mat(QByteArray array);
    QVector<double> mat2QVector(cv::Mat& mat);

signals:
    void newImage(QVector<QVector2D> observations);

private slots:
    void newSonarData(SonarReturnData data);

private:
    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;
};

#endif // SONARECHOFILTER_H
