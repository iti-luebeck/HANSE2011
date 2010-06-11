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
class Form_SonarLocalization;

class SonarParticleFilter : public QObject
{
Q_OBJECT

friend class Form_SonarLocalization;

public:

    SonarParticleFilter(Module_SonarLocalization* sonar, SonarEchoFilter *filter);

    QVector<QVector4D> getParticles();

signals:
    void newPosition(QVector3D position);

public slots:

private:
    Log4Qt::Logger *logger;
    const static int N = 2000;
    const static double DISTANCE_CUTOFF = 10000;

    QVector3D controlVariance;
    QVector3D initialVariance;

    Module_SonarLocalization* sonar;
    SonarEchoFilter *filter;
    QVector<QVector4D> particles;

    QVector<QVector2D> mapPoints;

    Mat map;
    Mat forbiddenArea;

    void loadMap();

    void reset();

    QVector3D getBestEstimate();

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
private slots:
    void newImage(QVector<QVector2D> observations);
};

#endif // SONARPARTICLEFILTER_H
