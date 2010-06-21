#include "sonarparticlefilter.h"

#include "module_sonarlocalization.h"
#include "sonarechofilter.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
using namespace std;

SonarParticleFilter::SonarParticleFilter(Module_SonarLocalization* sonar, SonarEchoFilter *filter)
    : controlVariance(0,0,0), initialVariance(0,0,0), s(sonar->getSettings())
{
    logger = Log4Qt::Logger::logger("SonarFilter");
    this->sonar = sonar;
    this->filter = filter;

    loadMap();

    reset();
}

void SonarParticleFilter::newImage(QList<QVector2D> observations)
{

    logger->debug("Queued new observation.");
    zList.append(observations);
    emit working(true);
    doNextUpdate();
    emit working(false);
}

void SonarParticleFilter::reset()
{
    QMutexLocker m(&particlesMutex);

    N = s.value("particleCount").toInt();
    if (N<1)
        N=100;

    particles.resize(N);

    QStringList initVarString = s.value("initVariance").toString().split(";");
    if (initVarString.size()<3) {
        logger->error("could not parse initVariance");
    } else {
        initialVariance = QVector3D(initVarString[0].toFloat(),
                                    initVarString[1].toFloat(),
                                    initVarString[2].toFloat());
    }

    QStringList controlVarString = s.value("controlVariance").toString().split(";");
    if (controlVarString.size()<3) {
        logger->error("could not parse controlVariance");
    } else {
        controlVariance = QVector3D(controlVarString[0].toFloat(),
                                    controlVarString[1].toFloat(),
                                    controlVarString[2].toFloat());
    }

    // TODO: initial pos!!!
    QVector3D initialPos = img2map(QVector2D(370,300)).toVector3D();
    initialPos.setZ(-M_PI/4+0.5);
    for (int i=0; i<N; i++) {
        particles[i] = sampleGauss(initialPos, initialVariance).toVector4D();
    }
}

void SonarParticleFilter::loadMap()
{
    Mat combinedMap = cv::imread(sonar->getSettings().value("mapFile").toString().toStdString(), 1);
    if (combinedMap.data == NULL) {
        logger->error("Could not read map!");
        return;
    }


    if (combinedMap.type() != CV_8UC3) {
        logger->debug("imread returned matrix with type "+QString::number(combinedMap.type()));
        logger->error("imread returned unexpected data type!");
        return;
    }

    vector<Mat> channels;
    channels.resize(3);

    split(combinedMap, &channels[0]);

    // forbidden area is red. allowed area is white. thus the forbidden area will have blue==0 and
    // the allowed have blue==255 (green would also work)
    forbiddenArea = channels[0]; // channels: BGR
    Mat map = channels[1];

    for (int r = 0; r<map.rows; r++) {
        for ( int c=0; c<map.cols; c++) {
            if (channels[0].at<unsigned char>(r,c)==0
                && channels[1].at<unsigned char>(r,c)==0
                && channels[2].at<unsigned char>(r,c)==0
                && qrand() < RAND_MAX/5 ) {  // TODo
                this->mapPoints.append(img2map(QVector2D(c,r)));
            }
        }
    }
}

QVector2D SonarParticleFilter::map2img(QVector2D mapPoint)
{
    return mapPoint/0.2;
}

QVector2D SonarParticleFilter::img2map(QVector2D imgPoint)
{
    return imgPoint*0.2;
}

QVector3D SonarParticleFilter::getBestEstimate()
{
    QMutexLocker m(&particlesMutex);

    // assume that the list is already sorted.
    return particles[0].toVector3D();
}

bool particleComparator(const QVector4D &p1, const QVector4D &p2)
{
    return p1.w() > p2.w();
}

void SonarParticleFilter::sortParticles()
{
     qSort(particles.begin(), particles.end(), particleComparator);
}

QVector3D SonarParticleFilter::sampleGauss(QVector3D mean, QVector3D variance)
{
    QVector3D g;
    g.setX(rand.gaussian(sqrt(variance.x())));
    g.setY(rand.gaussian(sqrt(variance.y())));
    g.setZ(rand.gaussian(sqrt(variance.z())));
    return mean + g;
}

double SonarParticleFilter::meassureObservation(QVector<QVector2D> observations)
{
    // TODO can likely be optimized by putting the mappoints into a quadtree or something...

    float cutoff = s.value("distanceCutoff").toFloat();
    float b = s.value("boltzmann").toFloat();

    double index = 1;
    foreach (QVector2D obsevationPoint, observations) {
        double bestVal = INFINITY;

        // compare with all elements in map
        foreach (QVector2D mapPoint, mapPoints) {

            double l = (mapPoint - obsevationPoint).length();

            if (l<bestVal)
                bestVal = l;
        }

        if (isinf(bestVal))
            bestVal = cutoff;

        index *= std::exp(-bestVal/b);
    }

    return index;

}

bool SonarParticleFilter::isPositionForbidden(QVector2D pos)
{
    QPoint imgPos = map2img(pos).toPoint();

    if (imgPos.x()<0 || imgPos.y()<0 || imgPos.x()>=forbiddenArea.cols || imgPos.y()>=forbiddenArea.rows )
        return true;

    return forbiddenArea.at<unsigned char>(imgPos.y(), imgPos.x())==0;
}

double SonarParticleFilter::max(QVector<double> v)
{
    double max=0;
    for (int i=0; i<N; i++)
        if (!isnan(v[i]) && v[i]>max)
            max=v[i];

    return max;
}

double SonarParticleFilter::min(QVector<double> v)
{
    double min=1;
    for (int i=0; i<N; i++)
        if (!isnan(v[i]) && v[i]<min)
            min=v[i];

    return min;
}

double SonarParticleFilter::sum(QVector<double> v)
{
    double sum=0;
    for (int i=0; i<N; i++)
        if (!isnan(v[i]))
            sum += v[i];

    return sum;
}

QVector<QVector4D> SonarParticleFilter::getParticles()
{
    QMutexLocker m(&particlesMutex);
    return particles;
}

void SonarParticleFilter::doNextUpdate()
{
    logger->debug("pressed next button.");

    if (zList.isEmpty())
        return;

    logger->debug("Dequeued new observation.");

    QList<QVector2D> observations = zList.takeFirst();

    particlesMutex.lock();
    QVector<QVector4D> oldParticles = particles;
    particlesMutex.unlock();

    if (observations.size()<s.value("imgMinPixels").toInt()) {
        logger->warn("not enough points. dropping meassurement.");
        return;
    }

    lastZ = observations;

    QVector<double> weights(N);

    logger->debug("Updating the particle filter...");

    // update paricles filter
    for(int i=0; i<N; i++) {

        QVector3D oldPos = oldParticles[i].toVector3D();

        QVector3D newPos = oldPos + sampleGauss(QVector3D(), controlVariance);
        oldParticles[i] = newPos.toVector4D();

        QVector<QVector2D> observationsTransformed(observations.size());

        // transform observation from local (particle relative) to global system
        QTransform rotM = QTransform().rotate(newPos.z()/M_PI*180) * QTransform().translate(newPos.x(), newPos.y());

        for(int j=0; j<observations.size(); j++) {
            QPointF p = rotM.map(observations[j].toPointF());
            observationsTransformed[j] = QVector2D(p);
        }

        if (isPositionForbidden(newPos.toVector2D())) {
            weights[i] = NAN;
        } else {
            // meassure it.
            double index = meassureObservation(observationsTransformed);
            weights[i] = index;

        }
        oldParticles[i].setW(weights[i]);
        logger->debug("Particle "+QString::number(i)+" has weight "+QString::number(weights[i]));
    }

    // normalize particle weights
    double minW = min(weights);
    for (int i=0; i<N; i++) {
        if (isnan(weights[i]))
            weights[i] = minW;
    }
    for (int i=0; i<N; i++) {
        weights[i] -= minW;
    }
    double sumW = sum(weights);
    QVector<double> cumsum(N+1);
    cumsum[N]=1; // safety entry on top
    for (int i=0; i<N; i++) {
        weights[i] /= sumW;
        oldParticles[i].setW(weights[i]);
        if (i==0) cumsum[i]=weights[i];
        else      cumsum[i]=cumsum[i-1]+weights[i];
    }

    if (fabs(cumsum[N-1]-1)> 10e-10) {
        logger->error("Cumsum has bad sum: "+QString::number(cumsum[N-1]));
    }

    // resample
    QVector<QVector4D> resampledParticles(N);
    for (int i=0; i<N; i++) {
        double r = 1.0*qrand()/RAND_MAX;
        int particle = 0;
        while (cumsum[particle] < r)
            particle++;

        if (particle==N)
            logger->error("BUG while resampling!");

        resampledParticles[i] = oldParticles[particle];
    }
    particlesMutex.lock();
    particles = resampledParticles;
    particlesMutex.unlock();

    // sort particles
    this->sortParticles();

    logger->debug("Updating the particle filter... DONE");

    emit newPosition(getBestEstimate());
}

QList<QVector2D> SonarParticleFilter::getLatestObservation()
{
    // TODO: mutex
    return lastZ;
}

QVector<QVector2D> SonarParticleFilter::getMapPoints()
{
    return mapPoints;
}
