#include "sonarechofilter.h"
#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>
#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>

using namespace cv;

SonarEchoFilter::SonarEchoFilter(Module_SonarLocalization* parent)
    : s(parent->getSettings())
{
    this->sloc = parent;

    logger = Log4Qt::Logger::logger("SonarFilter");

    reset();
}

/**
  * do some preprocessing on newly received sonar data.
  * try to find the wall, and put the wall position in a queue.
  * if we found a sufficiently long consequtive piece of wall,
  * group them in an image and send it off to the particle filter.
  */
void SonarEchoFilter::newSonarData(SonarReturnData data)
{
    if (!sloc->isEnabled())
        return;

    QByteArray byteEcho = data.getEchoData();
    
    if (byteEcho.size() != 252)
        logger->warn("Received " + QString::number(N)+ " sonar echos: Didn't test this!");

    // throw away the last two samples to make some calculations easier
    byteEcho.chop(byteEcho.size()-N);
    Mat echo = byteArray2Mat(byteEcho);

    // normalize
    echo /= MAX;

    // filter out noise etc.
    Mat echoFiltered = filterEcho(data,echo);

    rawHistory[data.switchCommand.time] = mat2QVector(echo);

    filteredHistory[data.switchCommand.time] = mat2QVector(echoFiltered);

    int K = findWall(data,echoFiltered);

    // pending later: SVM

    // pending later: Blob-Filter

    if (K<0) {
        darknessCount++;
    } else {
        logger->trace("found peak at "+QString::number(K));
        darknessCount=0;
        kHistory[data.switchCommand.time]=K;

        localKlist.append(K);
        localKlistHeading.append(data.getHeadPosition());
        localKlistID.append(currentID++);
    }

    if (localKlistHeading.size()>1) {
        int i = localKlistHeading.size()-1;
        double delta = localKlistHeading[i]-localKlistHeading[i-1];
        if (delta > 180)
            delta -= 360;
        if (delta < -180)
            delta += 360;

        swipedArea += fabs(delta);
    }

    // todo: this must be absolutely robust!!
    if (localKlist.size()>0 && (darknessCount>=s.value("darknessCnt").toInt() || swipedArea>s.value("swipedArea").toInt())) {
        // connect Ks until we have a closed image
        // transform from polar coordinates int euclid coordinates
        // form lines between points

        QList<QVector2D> posArray;

        posArray.clear();
        for(int i=0; i<localKlist.size(); i++) {
            double x,y;
            int range = 50;
            x = cos(localKlistHeading[i]/180*M_PI)*localKlist[i]/N*range;
            y = sin(localKlistHeading[i]/180*M_PI)*localKlist[i]/N*range;

            // connect points: XXX: buggy
//            if (i>0 && localKlistID[i]==localKlistID[i-1]+1) {
//                for (int k=localKlist[i-1]; k<=localKlist[i]; k++) {
//                    double ax = cos(localKlistHeading[i]/180*M_PI)*k/N*range;
//                    double ay = sin(localKlistHeading[i]/180*M_PI)*k/N*range;
//                    this->posArrayX.append(ax);
//                    this->posArrayY.append(ay);
//                }
//            }

            if (sqrt(x*x+y*y)>10) // ahhhh: evil heuristic!
                posArray.append(QVector2D(x,y)); // TODO mirror then adaptively
        }
        localKlist.clear();
        localKlistHeading.clear();
        localKlistID.clear();
        swipedArea = 0;
        darknessCount=0;

        logger->debug("Finished new partial echo image");

        // voila! we have a filtered sonar image
        emit newImage(posArray);

    }
}

Mat SonarEchoFilter::filterEcho(SonarReturnData data, const Mat& echo)
{
    Mat echoFiltered = Mat::zeros(1,N,CV_32F);

    QVector<double> thresh;

    int wSize = 1; // fixed!!!

    // remove noise from signal
    for(int i=wSize; i<N-wSize; i++) {
        Mat window = echo.colRange(i-wSize, i+wSize);

        // [ 0.1065    0.7870    0.1065 ] = sum(fspecial('gaussian'))
        float gF = s.value("gaussFactor").toFloat();
        echoFiltered.at<float>(0,i) =  window.at<float>(0,0)*(1-gF)/2
                                     + window.at<float>(0,1)*gF
                                     + window.at<float>(0,2)*(1-gF)/2;

        //GAIN=16: sensorNoiseThresh=max((7/20)*((1:250)-50),0);
        // TODO: either adapt or settings!
        float cutOff=0;
        if (data.switchCommand.startGain==15)
            cutOff = (7.0/20)*(i-50)/127;
//        else
//            logger->error("Unknown gain: "+QString::number(data.startGain));

        if (cutOff<0)
            cutOff = 0;

        thresh.append(cutOff);

        float newVal = echoFiltered.at<float>(0,i) - cutOff;
        if (newVal<0)
            newVal = 0;

        // normalize to 1 as maximum
        echoFiltered.at<float>(0,i) = newVal;
    }

    threshHistory[data.switchCommand.time] = thresh;
    return echoFiltered;
}

int SonarEchoFilter::findWall(SonarReturnData data,const Mat& echo)
{
    // find last maximum

    int wSize=s.value("wallWindowSize").toInt();

    QVector<double> varHist(N);
    QVector<double> meanHist(N);

    int K = -1;

    for(int j=N-wSize-1; j>=wSize; j--) {

        // window around the point we're looking at
        Mat window = echo.colRange(j-wSize, j+wSize);

        Scalar mean;
        Scalar stdDev;

        // calc stdDev inside window
        meanStdDev(window, mean,stdDev);
        float stdDevInWindow = stdDev[0]*stdDev[0];

        varHist[j]=stdDevInWindow;

        // TODO: fiddle with TH, or move it a little bit to the left
        bool largePeak = mean[0]>s.value("largePeakTH").toFloat();

        // calc mean in area behind our current pos.
        Mat prev = echo.colRange(j+wSize,echo.cols-1);
        meanStdDev(prev, mean, stdDev);
        float meanBehind = mean[0];

        meanHist[j]=meanBehind;

        // take first peak found.
        if (stdDevInWindow > s.value("varTH").toFloat()
            && meanBehind<s.value("meanBehindTH").toFloat()
            && largePeak && K<0) {
            K=j;
        }

    }

    // we could optimize some more, but it shouldn't matter
    if (DEBUG) {
        varHistory[data.switchCommand.time] = varHist;
        meanHistory[data.switchCommand.time] = meanHist;
    }
    return K;
}

void SonarEchoFilter::reset()
{
    this->DEBUG = s.value("DEBUG").toBool();

    this->darknessCount = 0;
    this->swipedArea = 0;
    this->currentID = 0;

    rawHistory.clear();
    filteredHistory.clear();
    kHistory.clear();
    threshHistory.clear();
    varHistory.clear();
    meanHistory.clear();

    localKlist.clear();
    localKlistHeading.clear();
    localKlistID.clear();
    currentID = -1;
    swipedArea = 0;
    darknessCount = 0;
}

Mat SonarEchoFilter::byteArray2Mat(QByteArray array)
{
    Mat m = Mat::zeros(1,array.size(),CV_32F);
    for(int i=0;i<array.size();i++) {
        m.at<float>(0,i) = array[i];
    }
    return m;
}

QVector<double> SonarEchoFilter::mat2QVector(Mat& mat)
{
    QVector<double> v;
    int w = mat.size().width;
    for(int i=0;i<w;i++) {
        v.append(mat.at<float>(0,i));
    }
    return v;
}