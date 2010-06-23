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

    SonarParticleFilter(Module_SonarLocalization* sonar, SonarEchoFilter *filter);

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

    QSettings& s;

    QQueue< QList<QVector2D> > zList;
    QList<QVector2D> lastZ;

    Log4Qt::Logger *logger;
    QMutex particlesMutex;
    int N;

    QVector3D controlVariance;
    QVector3D initialVariance;

    Module_SonarLocalization* sonar;
    SonarEchoFilter *filter;
    QVector<QVector4D> particles;

    QVector<QVector2D> mapPoints;

    Mat forbiddenArea;

    void loadMap();

    /* sort particles in descending quality */
    void sortParticles();

    cv::RNG rand;

    QVector3D sampleGauss(QVector3D mean, QVector3D variance);

    QVector2D map2img(QVector2D mapPoint);
    QVector2D img2map(QVector2D imgPoint);

    double meassureObservation(QVector<QVector2D> observations);

    bool isPositionForbidden(QVector2D pos);

    double max(QVector<double>);
    double min(QVector<double>);
    double sum(QVector<double>);

    void updateParticleFilter(QList<QVector2D> observations);
private slots:
    void newImage(QList<QVector2D> observations);
};

#endif // SONARPARTICLEFILTER_H
