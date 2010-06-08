#include "module_sonarlocalization.h"

#include <QtGui>
#include "form_sonarlocalization.h"

#include <opencv/cv.h>

#include <Module_ScanningSonar/module_scanningsonar.h>

using namespace cv;

Module_SonarLocalization::Module_SonarLocalization(QString id, Module_ScanningSonar *sonar)
    : RobotModule(id)
{
    this->sonar = sonar;

    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), this, SLOT(newSonarData(SonarReturnData)));
}

void Module_SonarLocalization::reset()
{
    RobotModule::reset();
}

void Module_SonarLocalization::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_SonarLocalization::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sonar);
    return ret;
}

QWidget* Module_SonarLocalization::createView(QWidget* parent)
{
    return new Form_SonarLocalization(parent, this);
}

void Module_SonarLocalization::newSonarData(SonarReturnData data)
{
    if (!isEnabled())
        return;

    QByteArray byteEcho = data.getEchoData();
    logger->debug("Received " + QString::number(byteEcho.size())+ " sonar echos.");

    // TODO: why 252??? it should be 250, or not?
    byteEcho.chop(byteEcho.size()-N);
    Mat echo = byteArray2Mat(byteEcho);

    echo = echo/127.0;

    // filter out noise etc.
    Mat echoFiltered = filterEcho(data,echo);

    rawHistory[data.dateTime] = mat2QVector(echo);

    filteredHistory[data.dateTime] = mat2QVector(echoFiltered);

    int K = findWall(data,echoFiltered);
    if (K<0) {
        // no peak found
    } else {
        logger->debug("found peak at "+QString::number(K));
        K_history.append(K);
        kHistory[data.dateTime]=K;
    }

    // connect with previous K
    // -> i have to keep Ks in memory

    // TODO: plot Ks in qgraphicsscene, and connect them

    // pending later: SVM

    // pending later: Blob-Filter

    // connect Ks until we have a closed image
    // transform from polar coordinates int euclid coordinates
    // form lines between points

    // voila! we have a filtered sonar image
    // now feed it into the particle filter

}

Mat Module_SonarLocalization::filterEcho(SonarReturnData data, const Mat& echo)
{
    Mat echoFiltered = Mat::zeros(1,N,CV_32F);

    QVector<double> thresh;

    int wSize = 1;

    // remove noise from signal
    for(int i=wSize; i<N-wSize; i++) {
        Mat window = echo.colRange(i-wSize, i+wSize);

        // [ 0.1065    0.7870    0.1065 ] = sum(fspecial('gaussian'))
        echoFiltered.at<float>(0,i) = 0.1065*window.at<float>(0,0)
                                     + 0.7870*window.at<float>(0,1)
                                     + 0.1065*window.at<float>(0,2);

        //GAIN=16: sensorNoiseThresh=max((7/20)*((1:250)-50),0);
        float cutOff=0;
        if (data.startGain==15)
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

    threshHistory[data.dateTime] = thresh;
    return echoFiltered;
}

int Module_SonarLocalization::findWall(SonarReturnData data,const Mat& echo)
{
    // find last maximum

    int wSize=3;

    QVector<double> varHist;
    QVector<double> meanHist;

    int K = -1;

    Mat var = Mat::zeros(1,N,CV_32F);
    for(int j=N-wSize-1; j>=wSize; j--) {

        // window around the point we're looking at
        Mat window = echo.colRange(j-wSize, j+wSize);

        Scalar mean;
        Scalar stdDev;

        // calc stdDev inside window
        meanStdDev(window, mean,stdDev);
        float stdDevInWindow = stdDev[0]*stdDev[0];
        var.at<float>(0,j) = stdDevInWindow;

        varHist.insert(0, stdDevInWindow);

        // TODO: heuristic
        bool largePeak = true;

        // calc mean in area behind our current pos.
        Mat prev = echo.colRange(j+wSize,echo.cols-1);
        meanStdDev(prev, mean, stdDev);
        float meanBehind = mean[0];

        meanHist.insert(0, meanBehind);

        // take first peak found.
        if (stdDevInWindow > stdDevInWindowTH && meanBehind<meanBehindTH && largePeak && K<0)
            K= j;

    }

    varHistory[data.dateTime] = varHist;
    meanHistory[data.dateTime] = meanHist;

    return K;
}

Mat Module_SonarLocalization::byteArray2Mat(QByteArray array)
{
    Mat m = Mat::zeros(1,array.size(),CV_32F);
    for(int i=0;i<array.size();i++) {
        m.at<float>(0,i) = array[i];
    }
    return m;
}

QVector<double> Module_SonarLocalization::mat2QVector(Mat& mat)
{
    QVector<double> v;
    int w = mat.size().width;
    for(int i=0;i<w;i++) {
        v.append(mat.at<float>(0,i));
    }
    return v;
}

