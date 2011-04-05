#ifndef SONARPARTICLEFILTER_H
#define SONARPARTICLEFILTER_H

#include <QObject>
#include <QtCore>
#include <QVector4D>
#include <QVector3D>
#include <QVector2D>
#include <opencv/cv.h>
#include <log4qt/logger.h>
#include <Framework/position.h>

using namespace cv;
using namespace std;

class Module_SonarLocalization;
class SonarEchoFilter;

class SonarParticleFilter : public QObject
{
Q_OBJECT

public:

    SonarParticleFilter(Module_SonarLocalization& sonar, SonarEchoFilter& filter);

    void reset();

    QVector<QVector4D> getParticles();
    QVector<QVector2D> getMapPoints();
    QList<QVector2D> getLatestObservation();
    int getParticleCount();

    QVector3D getBestEstimate();

    void setLocalization(QVector2D position);

signals:
    void newPosition(QVector3D position);
    void working(bool);

private:

//    QSettings& s;

    QQueue< QList<QVector2D> > zList;
    QList<QVector2D> lastZ;

    Log4Qt::Logger *logger;
    QMutex particlesMutex;
    int N;

    QVector3D controlVariance;
    QVector3D initialVariance;

    Module_SonarLocalization& sonar;
    SonarEchoFilter& filter;
    QVector<QVector4D> particles;

    QVector<QVector2D> mapPoints;
    cv::flann::Index* mapPointsFlann;
    Mat* mapPointsMat;

    Mat forbiddenArea;

    void loadMap();

    /* sort particles in descending quality */
    void sortParticles();

    cv::RNG rand;

    QVector3D sampleGauss(const QVector3D& mean, const QVector3D& variance);
    double sampleGauss(double m, double sigma);
    double sampleUni(double min, double max);

    QVector2D map2img(const QVector2D& mapPoint);
    QVector2D img2map(const QVector2D& imgPoint);

    double meassureObservation(const QVector<QVector2D>& observations);

    bool isPositionForbidden(const QVector2D& pos);

    double max(const QVector<double>&);
    double min(const QVector<double>&);
    double sum(const QVector<double>&);

    void updateParticleFilter(const QList<QVector2D>& observations);

    void addToList(QVector<QVector2D>& list, const QVector2D p);

private slots:
    void newImage(QList<QVector2D> observations);
};

#endif // SONARPARTICLEFILTER_H
