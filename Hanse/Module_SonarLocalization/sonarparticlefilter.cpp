#include "sonarparticlefilter.h"
#include "module_sonarlocalization.h"
#include "sonarechofilter.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Framework/Angles.h>

using namespace cv;
using namespace std;

SonarParticleFilter::SonarParticleFilter(Module_SonarLocalization& sonar, Module_XsensMTi *mti, SonarEchoFilter& filter)
    :
      particlesMutex(QMutex::Recursive),
      controlVariance(0,0,0),
      initialVariance(0,0,0),
      sonar(sonar),
      filter(filter)
{
    this->mti = mti;

    logger = Log4Qt::Logger::logger("SonarFilter");

    logger->debug("ParticleFilter constructor");

    reset();

    QObject::connect(this, SIGNAL(newPosition(QVector3D)), &sonar, SLOT(particleFilterDone(QVector3D)));
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

    loadMap();

    N = sonar.getSettingsValue("particleCount").toInt();
    if (N<1)
        N=100;

    particles.resize(N);

    QStringList initVarString = sonar.getSettingsValue("initVariance").toString().split(";");
    if (initVarString.size()<3) {
        logger->error("could not parse initVariance");
    } else {
        initialVariance = QVector3D(initVarString[0].toFloat(),
                                    initVarString[1].toFloat(),
                                    initVarString[2].toFloat());
    }

    QStringList controlVarString = sonar.getSettingsValue("controlVariance").toString().split(";");
    if (controlVarString.size()<3) {
        logger->error("could not parse controlVariance");
    } else {
        controlVariance = QVector3D(controlVarString[0].toFloat(),
                                    controlVarString[1].toFloat(),
                                    controlVarString[2].toFloat());
    }

    QVector3D initialPos = QVector3D(sonar.getSettingsValue("savedPosition").toPointF());
    for (int i=0; i<N; i++) {
        particles[i] = SonarParticle(sampleGauss(initialPos.x(), initialVariance.x()),
                                     sampleGauss(initialPos.y(), initialVariance.y()),
                                     sampleGauss(initialPos.z(), initialVariance.z()));
    }

    lastCompassHeading = -10000;

    file = new QFile("loc.txt");
    stream = NULL;
    if (file->open(QIODevice::WriteOnly)) {
        stream = new QTextStream(file);
        stream->setRealNumberNotation(QTextStream::FixedNotation);
    }

    lastMatchRatio = 0;
}

void SonarParticleFilter::loadMap()
{
    mapLoaded = false;
    mapPointsMat.release();

    if (!QFile(sonar.getSettingsValue("mapFile").toString()).exists()) {
        logger->error("No localization map found!");
        return;
    }
    logger->debug("Loading sonar map");
    IplImage* img = cvLoadImage(sonar.getSettingsValue("mapFile").toString().toStdString().c_str(), 1);
    if (!img) {
        logger->error("Could not read map file!");
        return;
    }

    Mat combinedMap = Mat(img);
    if (!combinedMap.data) {
        logger->error("Could not read map!");
        return;
    }

    if (combinedMap.type() != CV_8UC3) {
        logger->debug("imread returned matrix with type "+QString::number(combinedMap.type()));
        logger->error("imread returned unexpected data type!");
        return;
    }

    Mat channels[3];

    split(combinedMap, channels);

    // forbidden area is red. allowed area is white. thus the forbidden area will have blue==0 and
    // the allowed have blue==255 (green would also work)
    forbiddenArea = channels[0]; // channels: BGR
    Mat map = channels[1];

    mapPoints.clear();
    for (int r = 0; r<map.rows; r++) {
        for ( int c=0; c<map.cols; c++) {
//            qDebug("%d %d %d", channels[0].at<unsigned char>(r,c), channels[1].at<unsigned char>(r,c), channels[2].at<unsigned char>(r,c));
            if ((channels[0].at<unsigned char>(r,c) < 100)
                && (channels[1].at<unsigned char>(r,c) < 100)
                && (channels[2].at<unsigned char>(r,c) < 100)) {
                addToList(mapPoints, img2map(QVector2D(c,r)), 0.5);
            }
        }
    }

    if (mapPoints.size()==0)
        return;

    // the flann data structure keeps referencing the original Mat object!!!
    mapPointsMat = Mat(Mat::zeros(mapPoints.size(),2,CV_32F));
    for(int i=0;i<mapPoints.size();i++) {
        mapPointsMat.at<float>(i,0) = mapPoints[i].x();
        mapPointsMat.at<float>(i,1) = mapPoints[i].y();
        // logger->trace("Adding point x="+QString::number(mapPointsMat.at<float>(i,0))
        //              +" y="+QString::number(mapPointsMat.at<float>(i,1)));
    }

    //this->mapPointsFlann = new cv::flann::Index(*mapPointsMat, cv::flann::KDTreeIndexParams(4));
    //this->mapPointsFlann = new cv::flann::Index(*mapPointsMat, cv::flann::LinearIndexParams());
    this->mapPointsFlann = new cv::flann::Index(mapPointsMat, cv::flann::AutotunedIndexParams());

    mapLoaded = true;

}

bool SonarParticleFilter::hasMap() {
    return mapLoaded;
}

void SonarParticleFilter::addToList(QList<QVector2D>& list, const QVector2D p, double Tdist)
{
    double minDist = 10000;
    foreach (QVector2D q, list) {
        minDist = qMin((q - p).length(), minDist);
    }

    if (minDist > Tdist) {
        list.append(p);
    }
}

QVector2D SonarParticleFilter::map2img(const QVector2D& mapPoint)
{
    //0.2
    qreal r = sonar.getSettingsValue("scaleMap").toReal();
    return mapPoint/r;
}

QVector2D SonarParticleFilter::img2map(const QVector2D& imgPoint)
{
    QVector2D ret = imgPoint;
    ret.setX(ret.x() - 1);
    ret.setY(ret.y() - 1);
    qreal r = sonar.getSettingsValue("scaleMap").toReal();
    return ret*r;
}

QVector3D SonarParticleFilter::getBestEstimate()
{
    QMutexLocker m(&particlesMutex);

    // assume that the list is already sorted.
    return QVector3D(particles[0].getX(), particles[0].getY(), particles[0].getTheta());
}

bool particleComparator(const SonarParticle &p1, const SonarParticle &p2)
{
    return p1.getWeight() > p2.getWeight();
}

void SonarParticleFilter::sortParticles()
{
     qSort(particles.begin(), particles.end(), particleComparator);
}

double SonarParticleFilter::sampleGauss(double m, double sigma)
{
    double x1 = 0, x2 = 0, w = 0;

    do {
            x1 = 2.0 * sampleUni(0, 1) - 1.0;
            x2 = 2.0 * sampleUni(0, 1) - 1.0;
            w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );

    w = sqrt( (-2.0 * log(w) ) / w );
    return m + sigma * (x1 * w);
}

double SonarParticleFilter::sampleUni(double min, double max)
{
    double a = (double)qrand();
    double r = (a + 1) / RAND_MAX;
    return min + (max - min)*r;
}

double SonarParticleFilter::meassureObservation(const QVector<QVector2D>& observations, const QList<QVector2D>& obsRelativeToRobot, double &matchRatio)
{
    if (!hasMap()) {
        return 0;
    }

    int N = observations.size();

    float sigma2 = sonar.getSettingsValue("observationVariance").toFloat();
    sigma2 *= sigma2;
    float cutoff2 = sonar.getSettingsValue("distanceCutoff").toFloat();
    cutoff2 *= cutoff2;

    // TODO: this data copying can be avoided by doing all math in opencv data structures
    Mat zPoints(N, 2, CV_32F);
    for (int i=0; i<N; i++) {
        zPoints.at<float>(i,0) = observations[i].x();
        zPoints.at<float>(i,1) = observations[i].y();
    }
    Mat indices = Mat::zeros(N, 1, CV_32S);
    Mat dists = Mat::zeros(N, 1, CV_32F);
    this->mapPointsFlann->knnSearch(zPoints, indices, dists, 1, cv::flann::SearchParams(32));
    double index = 0;
    float numMatches = 0;
    for (int i = 0; i < N; i++) {
        double obsDist = obsRelativeToRobot[i].length();
        double diff = dists.at<float>(i,0);
        if (diff > cutoff2) {
            diff = cutoff2;
        } else {
            numMatches++;
        }
        index += diff / ((((100 - obsDist) / 100)) * sigma2);
    }
    index = exp(-0.5 * index);
//    index = index / (N * cutoff2);
//    index = 1 - index;

//    double worstPossible = exp(-0.5 * N * cutoff2);

    matchRatio = numMatches / N;

    return index;
}

double SonarParticleFilter::meassureMatches(const QVector<QVector2D>& observations)
{
    if (!hasMap()) {
        return 0;
    }

    int N = observations.size();

    float cutoff2 = sonar.getSettingsValue("distanceCutoff").toFloat();
    cutoff2 *= cutoff2;

    // TODO: this data copying can be avoided by doing all math in opencv data structures
    Mat zPoints(N, 2, CV_32F);
    for (int i=0; i<N; i++) {
        zPoints.at<float>(i,0) = observations[i].x();
        zPoints.at<float>(i,1) = observations[i].y();
    }
    Mat indices = Mat::zeros(N, 1, CV_32S);
    Mat dists = Mat::zeros(N, 1, CV_32F);
    this->mapPointsFlann->knnSearch(zPoints, indices, dists, 1, cv::flann::SearchParams(32));
    double index = 0;
    for (int i=0; i<N; i++) {
//        QVector2D diff = mapPoints[indices.at<int>(i,0)] - observations[i];
        double diff = dists.at<float>(i,0);
        if (diff < cutoff2) {
            index++;
        }
    }

    return index / (double)N;
}

bool SonarParticleFilter::isPositionForbidden(const QVector2D& pos)
{
    if (!hasMap()) {
        return false;
    }

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

QVector<SonarParticle> SonarParticleFilter::getParticles()
{
    QMutexLocker m(&particlesMutex);
    return particles;
}

void SonarParticleFilter::updateParticleFilter(QList<QVector2D> observations)
{
    //qDebug() << "obs length = " << observations.length();
//    logger->debug("pressed next button.");

    particlesMutex.lock();
    QVector<SonarParticle> oldParticles = particles;
    particlesMutex.unlock();

    double refTime = sonar.sonarEchoFilter().lastObservationTime;

    // Ignore observation that are close to another.
    QList<QVector2D> filteredObservations;
    for (int i = 0; i < observations.size(); i++) {
        addToList(filteredObservations, observations[i], sonar.getSettingsValue("min obs dist", 2.0).toDouble());
    }

    particlesMutex.lock();
    lastZ = filteredObservations;
    particlesMutex.unlock();

    QVector<double> weights(N);

    //logger->debug("Updating the particle filter...");

    // Update orientation with information from Xsens MTi.
    float diffHeading = 0;
    if (sonar.getSettingsValue("use xsens").toBool()) {
        float compassHeading = sonar.sonarEchoFilter().getLastObservationHeading();
        if (lastCompassHeading < -1000) {
            lastCompassHeading = compassHeading;
        } else {
            diffHeading = Angles::deg2pi(compassHeading - lastCompassHeading);
            sonar.addData("xsens heading difference", Angles::pi2pi(diffHeading));
            lastCompassHeading = compassHeading;
        }
    }

    if (observations.size() < sonar.getSettingsValue("imgMinPixels").toInt()) {
        logger->warn("not enough points. dropping meassurement.");
        return;
    }

    // Filter observations so that they are somewhat spaced.
    QList<QVector2D> tmpList = observations;
    observations.clear();
    for (int i = 0; i < tmpList.size(); i++) {
        addToList(observations, tmpList[i], 2.0);
    }

    particlesMutex.lock();
    lastZ = observations;
    particlesMutex.unlock();

//    QVector<double> weights(N);

   // logger->debug("Updating the particle filter...");

    // update paricles filter
    for (int i = 0; i < N; i++) {
        SonarParticle particle = oldParticles[i];

        // Add heading difference.
        float newHeading = Angles::pi2pi(particle.getTheta() - diffHeading);
        particle.setTheta(newHeading);

        // Randomize pose.
        particle.setX(sampleGauss(particle.getX(), controlVariance.x()));
        particle.setY(sampleGauss(particle.getY(), controlVariance.y()));
        particle.setTheta(sampleGauss(particle.getTheta(), controlVariance.z()));

        QVector<QVector2D> observationsTransformed(filteredObservations.size());

        // transform observation from local (particle relative) to global system
        QTransform rotM = QTransform().rotate(Angles::pi2deg(particle.getTheta())) * QTransform().translate(particle.getX(), particle.getY());

        for (int j = 0; j < filteredObservations.size(); j++) {
            QPointF p = rotM.map(filteredObservations[j].toPointF());
            observationsTransformed[j] = QVector2D(p);
        }

        if (isPositionForbidden(QVector2D(particle.getX(), particle.getY()))) {
            weights[i] = 0;
            particle.setMatchRatio(0);
        } else {
            double matchRatio = 0;
            double w = meassureObservation(observationsTransformed, filteredObservations, matchRatio);
            particle.setMatchRatio(matchRatio);
            weights[i] = w;
        }
        particle.setWeight(weights[i]);
        oldParticles[i] = particle;
    }

    double sumW = sum(weights);
    QVector<double> cumsum(N+1);
    cumsum[N]=1; // safety entry on top
    for (int i=0; i<N; i++) {
        weights[i] /= sumW;
        oldParticles[i].setWeight(weights[i]);
        if (i==0) cumsum[i]=weights[i];
        else      cumsum[i]=cumsum[i-1]+weights[i];
    }

    if (fabs(cumsum[N-1]-1)> 10e-10) {
        logger->error("Cumsum has bad sum: "+QString::number(cumsum[N-1]));
    }

    // resample
    QVector<SonarParticle> resampledParticles(N);
    for (int i = 0; i < N; i++) {
        double r = 1.0*qrand()/((double)RAND_MAX + 1.0);
        int particle = 0;
        while (cumsum[particle] < r)
            particle++;

        if (particle >= N) {
            logger->error("BUG while resampling!");
            particle = N - 1;
        }

        resampledParticles[i] = oldParticles[particle];
    }
    particlesMutex.lock();
    particles = resampledParticles;
    particlesMutex.unlock();

    // sort particles
    this->sortParticles();

   // logger->debug("Updating the particle filter... DONE");

    this->sonar.setSettingsValue("savedPosition", getBestEstimate().toPointF());

    particlesMutex.lock();
    QVector3D bestPos = getBestEstimate();
    QVector<QVector2D> bestObservations(observations.size());

    // transform observation from local (particle relative) to global system
    QTransform rotM = QTransform().rotate(Angles::pi2deg(bestPos.z())) * QTransform().translate(bestPos.x(), bestPos.y());

    for(int j=0; j<observations.size(); j++) {
        QPointF p = rotM.map(observations[j].toPointF());
        bestObservations[j] = QVector2D(p);
    }
    lastMatchRatio = meassureMatches(bestObservations);
    if (stream) {
        *stream << refTime << " " << bestPos.x() << " " << bestPos.y() << " " << lastMatchRatio << endl;
    }
    particlesMutex.unlock();

    sonar.addData("x", bestPos.x());
    sonar.addData("y", bestPos.y());
    sonar.addData("theta", bestPos.z());
    sonar.addData("weight", particles[0].getWeight());
    sonar.addData("match ratio", particles[0].getMatchRatio());

    emit newPosition(getBestEstimate());
}

QList<QVector2D> SonarParticleFilter::getLatestObservation()
{
    QMutexLocker m(&particlesMutex);
    return lastZ;
}

QList<QVector2D> SonarParticleFilter::getMapPoints()
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
        particles[i] = SonarParticle(sampleGauss(initialPos.x(), initialVariance.x()),
                                     sampleGauss(initialPos.y(), initialVariance.y()),
                                     sampleGauss(initialPos.z(), initialVariance.z()));
    }

    //re-evaluate on last observation; sort particles; send signal
    this->updateParticleFilter(lastZ);
}
