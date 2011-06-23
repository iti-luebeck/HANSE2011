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
class Module_XsensMTi;
class SonarEchoFilter;

class SonarParticle
{
public:
    SonarParticle()
    {
        this->x = .0f;
        this->y = .0f;
        this->theta = .0f;
        this->weight = .0f;
        this->matchRatio = .0f;
    }
    SonarParticle(float x, float y, float theta)
    {
        this->x = x;
        this->y = y;
        this->theta = theta;
        this->weight = .0f;
        this->matchRatio = .0f;
    }
    SonarParticle(const SonarParticle &copy)
    {
        this->x = copy.x;
        this->y = copy.y;
        this->theta = copy.theta;
        this->weight = copy.weight;
        this->matchRatio = copy.matchRatio;
    }

    SonarParticle& operator=(const SonarParticle& copy)
    {
        this->x = copy.x;
        this->y = copy.y;
        this->theta = copy.theta;
        this->weight = copy.weight;
        this->matchRatio = copy.matchRatio;
        return *this;
    }

    float getX() const { return x; }
    void setX(float x) { this->x = x; }

    float getY() const { return y; }
    void setY(float y) { this->y = y; }

    float getTheta() const { return theta; }
    void setTheta(float theta) { this->theta = theta; }

    float getWeight() const { return weight; }
    void setWeight(float weight) { this->weight = weight; }

    float getMatchRatio() const { return matchRatio; }
    void setMatchRatio(float ratio) { this->matchRatio = ratio; }

private:
    float x;
    float y;
    float theta;
    float weight;
    float matchRatio;
};

class SonarParticleFilter : public QObject
{
Q_OBJECT

public:

    SonarParticleFilter(Module_SonarLocalization& sonar, Module_XsensMTi *mti, SonarEchoFilter& filter);

    void reset();

    QVector<SonarParticle> getParticles();
    QList<QVector2D> getMapPoints();
    QList<QVector2D> getLatestObservation();
    int getParticleCount();

    QVector3D getBestEstimate();

    void setLocalization(QVector2D position);

    bool hasMap();

signals:
    void newPosition(QVector3D position);
    void working(bool);

private:
    QList<QVector2D> lastZ;

    Log4Qt::Logger *logger;
    QMutex particlesMutex;
    int N;

    QVector3D controlVariance;
    QVector3D initialVariance;

    Module_SonarLocalization& sonar;
    Module_XsensMTi *mti;

    SonarEchoFilter& filter;
    QVector<SonarParticle> particles;

    QList<QVector2D> mapPoints;
    cv::flann::Index* mapPointsFlann;
    Mat mapPointsMat;
    bool mapLoaded;

    Mat forbiddenArea;

    float lastCompassHeading;

    void loadMap();

    /* sort particles in descending quality */
    void sortParticles();

    cv::RNG rand;

    double sampleGauss(double m, double sigma);
    double sampleUni(double min, double max);

    QVector2D map2img(const QVector2D& mapPoint);
    QVector2D img2map(const QVector2D& imgPoint);

    double meassureObservation(const QVector<QVector2D>& observations, const QList<QVector2D>& obsRelativeToRobot, double &matchRatio);
    double meassureMatches(const QVector<QVector2D>& observations);

    bool isPositionForbidden(const QVector2D& pos);

    double max(const QVector<double>&);
    double min(const QVector<double>&);
    double sum(const QVector<double>&);

    void updateParticleFilter(QList<QVector2D> observations);

    void addToList(QList<QVector2D>& list, const QVector2D p, double Tdist);

    QFile* file;
    QTextStream* stream;
    double lastMatchRatio;

private slots:
    void newImage(QList<QVector2D> observations);
};

#endif // SONARPARTICLEFILTER_H
