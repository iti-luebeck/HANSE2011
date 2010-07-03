#include "sonarparticlefilter.h"
#include "module_sonarlocalization.h"
#include "sonarechofilter.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

using namespace cv;
using namespace std;

SonarParticleFilter::SonarParticleFilter(Module_SonarLocalization* sonar, SonarEchoFilter *filter)
    : controlVariance(0,0,0), initialVariance(0,0,0),
      s(sonar->getSettings()), particlesMutex(QMutex::Recursive)
{
    logger = Log4Qt::Logger::logger("SonarFilter");
    this->sonar = sonar;
    this->filter = filter;

    loadMap();

    reset();
}

void SonarParticleFilter::newImage(QList<QVector2D> observations)
{
    // TODO: find out somehow, when there is a jam
    emit working(true);
    updateParticleFilter(observations);
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

    QVector3D initialPos = QVector3D(sonar->getSettings().value("savedPosition").toPointF());
    for (int i=0; i<N; i++) {
        particles[i] = sampleGauss(initialPos, initialVariance).toVector4D();
    }
}

void SonarParticleFilter::loadMap()
{
    logger->debug("Loading sonar map");
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
                && channels[2].at<unsigned char>(r,c)==0) {
                addToList(mapPoints, img2map(QVector2D(c,r)));
            }
        }
    }

    // the flann data structure keeps referencing the original Mat object!!!
    mapPointsMat = new Mat(Mat::zeros(mapPoints.size(),2,CV_32F));
    for(int i=0;i<mapPoints.size();i++) {
        mapPointsMat->at<float>(i,0) = mapPoints[i].x();
        mapPointsMat->at<float>(i,1) = mapPoints[i].y();
        logger->trace("Adding point x="+QString::number(mapPointsMat->at<float>(i,0))
                      +" y="+QString::number(mapPointsMat->at<float>(i,1)));
    }

    //this->mapPointsFlann = new cv::flann::Index(*mapPointsMat, cv::flann::KDTreeIndexParams(4));
    //this->mapPointsFlann = new cv::flann::Index(*mapPointsMat, cv::flann::LinearIndexParams());
    this->mapPointsFlann = new cv::flann::Index(*mapPointsMat, cv::flann::AutotunedIndexParams());

}

void SonarParticleFilter::addToList(QVector<QVector2D>& list, const QVector2D p)
{
    if (list.size()==0) {
        list.append(p);
        return;
    }

    QVector2D& q = list[list.size()-1];
    if ((q-p).length()>0.5)
        list.append(p);
}

QVector2D SonarParticleFilter::map2img(const QVector2D& mapPoint)
{
    return mapPoint/0.2;
}

QVector2D SonarParticleFilter::img2map(const QVector2D& imgPoint)
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

QVector3D SonarParticleFilter::sampleGauss(const QVector3D& mean, const QVector3D& variance)
{
    QVector3D g;
    g.setX(rand.gaussian(sqrt(variance.x())));
    g.setY(rand.gaussian(sqrt(variance.y())));
    g.setZ(rand.gaussian(sqrt(variance.z())));
    return mean + g;
}

double SonarParticleFilter::meassureObservation(const QVector<QVector2D>& observations)
{
    int N = observations.size();
    float b = s.value("boltzmann").toFloat();

    // TODO: this data copying can be avoided by doing all math in opencv data structures
    Mat zPoints(N, 2, CV_32F);
    for (int i=0; i<N; i++) {
        zPoints.at<float>(i,0) = observations[i].x();
        zPoints.at<float>(i,1) = observations[i].y();
    }
    Mat indices = Mat::zeros(N, 1, CV_32S);
    Mat dists = Mat::zeros(N, 1, CV_32F);
    this->mapPointsFlann->knnSearch(zPoints, indices, dists, 1, cv::flann::SearchParams(32));
    double index = 1;
    for (int i=0; i<N; i++) {
        double bestVal = sqrt(dists.at<float>(i,0));
        index *= std::exp(-bestVal/b);
    }

    return index;
}

bool SonarParticleFilter::isPositionForbidden(const QVector2D& pos)
{
    QPoint imgPos = map2img(pos).toPoint();

    if (imgPos.x()<0 || imgPos.y()<0 || imgPos.x()>=forbiddenArea.cols || imgPos.y()>=forbiddenArea.rows )
        return true;

    return forbiddenArea.at<unsigned char>(imgPos.y(), imgPos.x())==0;
}

double SonarParticleFilter::max(const QVector<double>& v)
{
    double max=0;
    for (int i=0; i<N; i++)
        if (!isnan(v[i]) && v[i]>max)
            max=v[i];

    return max;
}

double SonarParticleFilter::min(const QVector<double>& v)
{
    double min=1;
    for (int i=0; i<N; i++)
        if (!isnan(v[i]) && v[i]<min)
            min=v[i];

    return min;
}

double SonarParticleFilter::sum(const QVector<double>& v)
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

void SonarParticleFilter::updateParticleFilter(const QList<QVector2D>& observations)
{
    logger->debug("pressed next button.");

    particlesMutex.lock();
    QVector<QVector4D> oldParticles = particles;
    particlesMutex.unlock();

    if (observations.size()<s.value("imgMinPixels").toInt()) {
        logger->warn("not enough points. dropping meassurement.");
        return;
    }

    particlesMutex.lock();
    lastZ = observations;
    particlesMutex.unlock();

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
        logger->trace("Particle "+QString::number(i)+" has weight "+QString::number(weights[i]));
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

    this->sonar->getSettings().setValue("savedPosition", getBestEstimate().toPointF());

    emit newPosition(getBestEstimate());
}

QList<QVector2D> SonarParticleFilter::getLatestObservation()
{
    QMutexLocker m(&particlesMutex);
    return lastZ;
}

QVector<QVector2D> SonarParticleFilter::getMapPoints()
{
    QMutexLocker m(&particlesMutex);
    return mapPoints;
}

int SonarParticleFilter::getParticleCount()
{
    return N;
}

void SonarParticleFilter::setLocalization(QVector2D position)
{
    QMutexLocker m(&particlesMutex);
    logger->info("Doing manual localization");

    QVector3D initialPos = position.toVector3D();
    for (int i=0; i<N; i++) {
        particles[i] = sampleGauss(initialPos, initialVariance).toVector4D();
    }

    //re-evaluate on last observation; sort particles; send signal
    this->updateParticleFilter(lastZ);
}
