#include "sonarparticlefilter.h"

#include "module_sonarlocalization.h"
#include "sonarechofilter.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
using namespace std;

SonarParticleFilter::SonarParticleFilter(Module_SonarLocalization* sonar, SonarEchoFilter *filter)
    : QObject(), controlVariance(1,1,1.5), initialVariance(150,150,1.5)
{
    logger = Log4Qt::Logger::logger("SonarFilter");
    this->sonar = sonar;
    this->filter = filter;

    qRegisterMetaType< QVector<QVector2D> >("QVector<QVector2D>");
    connect(filter, SIGNAL(newImage(QVector<QVector2D>)), this, SLOT(newImage(QVector<QVector2D>)));

    // LOAD MAP (walls and forbidden areas)

    loadMap();

    reset();
}

void SonarParticleFilter::newImage(QVector<QVector2D> observations)
{

    QVector<double> weights(N);

    logger->debug("Updating the particle filter...");

    // update paricles filter
    for(int i=0; i<N; i++) {

        QVector3D oldPos = particles[i].toVector3D();

        QVector3D newPos = oldPos + sampleGauss(QVector3D(), controlVariance);

        QVector<QVector2D> observationsTransformed;

        // transform observation from local (particle relative) to global system
        QTransform rotM;
        rotM.rotate(newPos.z());
        rotM.translate(newPos.x(), newPos.y());

        for(int j=0; j<observations.size(); j++) {
            QPointF p = rotM.map(observations[j].toPointF());
            observationsTransformed.append(QVector2D(p));
        }

        if (isPositionForbidden(newPos.toVector2D())) {
            weights[i] = NAN;
        } else {
            // meassure it.
            double index = meassureObservation(observationsTransformed);
            weights[i] = index;
        }
    }


    // normalize particle weights
    double maxW = max(weights);
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

    if (abs(cumsum[N-1]-1)> 10e-10) {
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

        resampledParticles.append(particles[particle]);
    }
    particles = resampledParticles;

    // sort particles
    //sort();

    logger->debug("Updating the particle filter... DONE");

    emit newPosition(getBestEstimate());

}

void SonarParticleFilter::reset()
{
    particles.resize(N);
    for (int i=0; i<N; i++) {
        particles[i] = sampleGauss(img2map(QVector2D(400,280)).toVector3D(), initialVariance).toVector4D();
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
    forbiddenArea = channels[0];
    map = channels[1];

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
    //return particles[0].toVector3D();
    double bestW = 0;
    QVector4D best;
    for(int i=0; i<N; i++) {
        if (particles[i].w()>bestW) {
            bestW = particles[i].w();
            best = particles[i];
        }
    }
    return best.toVector3D();
}

void SonarParticleFilter::sortParticles()
{
 // TODO
}

QVector3D SonarParticleFilter::sampleGauss(QVector3D mean, QVector3D variance)
{
    QVector3D g;
    g.setX(rand.gaussian(sqrt(variance.x())));
    g.setY(rand.gaussian(sqrt(variance.y())));
    g.setZ(rand.gaussian(sqrt(variance.z())));
    return mean + g;
}

double SonarParticleFilter::meassureObservation(QVector<QVector2D>observations)
{
    double index = 1;
    for (int i=0; i<observations.size(); i++) {
        QVector2D obsevationPoint = observations[i];
        double bestVal = INFINITY;
        // compare with all elements in map
        for (int j=0; j<this->mapPoints.size(); j++) {
            QVector2D mapPoint = mapPoints[j];
            double l = (mapPoint - obsevationPoint).length();
            if (l<bestVal)
                bestVal = l;
        }

        if (isinf(bestVal))
            bestVal = DISTANCE_CUTOFF;

        index *= std::exp(-bestVal);
    }

    return index;

}

bool SonarParticleFilter::isPositionForbidden(QVector2D pos)
{
    return false; // TODO
}

double SonarParticleFilter::max(QVector<double> v)
{
    double max=v[0];
    for (int i=0; i<N; i++)
        if (!isnan(v[i]) && v[i]>max)
            max=v[i];

    return max;
}

double SonarParticleFilter::min(QVector<double> v)
{
    double min=v[0];
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
    return particles;
}
