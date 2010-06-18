#ifndef SONARECHOFILTER_H
#define SONARECHOFILTER_H

#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>
#include <log4qt/logger.h>

class Module_SonarLocalization;

class SonarEchoFilter: public QObject
{
    Q_OBJECT

public:
    SonarEchoFilter(Module_SonarLocalization* sonar);

    const static int N = 250;

    void reset();

    QMap<QDateTime,QVector<double> > rawHistory;
    QMap<QDateTime,QVector<double> > filteredHistory;
    QMap<QDateTime, int > kHistory;
    QMap<QDateTime, QVector<double> > threshHistory;
    QMap<QDateTime, QVector<double> > varHistory;
    QMap<QDateTime, QVector<double> > meanHistory;

signals:
    void newImage(QList<QVector2D> observations);

private slots:
    void newSonarData(SonarReturnData data);

private:
    // max value of any samples coming from the sonar
    const static float MAX = 127;

    QList<int> localKlist;
    QList<double> localKlistHeading;
    QList<int> localKlistID;
    int currentID;
    double swipedArea;
    int darknessCount;

    bool DEBUG;
    QSettings& s;

    Module_SonarLocalization* sloc;

    /**
      * Logger instance for this module
      */
    Log4Qt::Logger *logger;

    cv::Mat filterEcho(SonarReturnData data,const cv::Mat& echo);
    int findWall(SonarReturnData data,const cv::Mat& echo);
    cv::Mat byteArray2Mat(QByteArray array);
    QVector<double> mat2QVector(cv::Mat& mat);

};

#endif // SONARECHOFILTER_H
