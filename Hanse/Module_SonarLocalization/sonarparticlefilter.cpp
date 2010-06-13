#include "sonarparticlefilter.h"

#include "module_sonarlocalization.h"
#include "sonarechofilter.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
using namespace std;

SonarParticleFilter::SonarParticleFilter(Module_SonarLocalization* sonar, SonarEchoFilter *filter)
    : QObject(), controlVariance(15,15,1.5), initialVariance(180,180,1.5)
{
    logger = Log4Qt::Logger::logger("SonarFilter");
    this->sonar = sonar;
    this->filter = filter;

    //QMutexLocker m(&particlesMutex);

    qRegisterMetaType< QVector<QVector2D> >("QVector<QVector2D>");
    connect(filter, SIGNAL(newImage(QVector<QVector2D>)), this, SLOT(newImage(QVector<QVector2D>)));

    // LOAD MAP (walls and forbidden areas)

    loadMap();

    reset();
}

void SonarParticleFilter::newImage(QVector<QVector2D> observations)
{

    logger->debug("Queued new observation.");
    zList.append(observations);
    //doNextUpdate();
}

void SonarParticleFilter::reset()
{
    QMutexLocker m(&particlesMutex);
    particles.resize(N);
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
                && qrand() < RAND_MAX/5 ) {
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
            bestVal = DISTANCE_CUTOFF;

        index *= std::exp(-bestVal/100);
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

    QVector<QVector2D> observations = zList.takeFirst();

    if (observations.size()<10) {
        logger->warn("not enough points. dropping meassurement.");
        return;
    }

    lastZ = observations;

    QVector<double> weights(N);

    logger->debug("Updating the particle filter...");

    // update paricles filter
    for(int i=0; i<N; i++) {

        QVector3D oldPos = particles[i].toVector3D();

        QVector3D newPos = oldPos + sampleGauss(QVector3D(), controlVariance);
        particles[i] = newPos.toVector4D();

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
        particles[i].setW(weights[i]);
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
        particles[i].setW(weights[i]);
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

        resampledParticles[i] = particles[particle];
    }
    particlesMutex.lock();
    particles = resampledParticles;
    particlesMutex.unlock();

    // sort particles
    this->sortParticles();

    logger->debug("Updating the particle filter... DONE");

    emit newPosition(getBestEstimate());
}
