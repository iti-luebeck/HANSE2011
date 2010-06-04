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

    QByteArray echo = data.getEchoData();
    logger->debug("Received " + QString::number(echo.size())+ " sonar echos.");

    // TODO: why 252??? it should be 250, or not?
    echo.chop(echo.size()-N);

    QVector<double> rawData;
    for(int i=0;i<N;i++)
        rawData.append(echo[i]);

    echoHistory[data.dateTime] = rawData;

    // filter out noise etc.
    QByteArray echoFiltered = filterEcho(data,echo);

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

    // pending later: SVM

    // pending later: Blob-Filter

    // connect Ks until we have a closed image
    // transform from polar coordinates int euclid coordinates
    // form lines between points

    // voila! we have a filtered sonar image
    // now feed it into the particle filter

}

QByteArray Module_SonarLocalization::filterEcho(SonarReturnData data,QByteArray echo)
{
    // TODO: low pass filter
    QByteArray echoFiltered = echo;

    QVector<double> thresh;

    // remove noise from signal
    for (int i=0; i<N; i++) {

        //GAIN=16: sensorNoiseThresh=max((7/20)*((1:250)-50),0);
        float cutOff=0;
        if (data.startGain==16)
            cutOff = (7.0/20)*(i-50);
//        else
//            logger->error("Unknown gain: "+QString::number(data.startGain));

        if (cutOff<0)
            cutOff = 0;

        thresh.append(cutOff);

        float newVal = echoFiltered[i] - cutOff;
        if (newVal<0)
            newVal = 0;

        echoFiltered[i] = int(newVal);
    }

    threshHistory[data.dateTime] = thresh;

    // TODO: do some kind of normalization?

    return echoFiltered;
}

int Module_SonarLocalization::findWall(SonarReturnData data,const QByteArray echo)
{
    // find last maximum

    int wSize=3;

    QVector<double> varHist;

    int K = -1;

    Mat var = Mat::zeros(N,1,CV_32F);
    for(int j=N-wSize-1; j>=wSize; j--) {

        // window around the point we're looking at
        Mat window = byteArray2Mat(echo.mid(j-wSize, 2*wSize+1));

        Scalar mean;
        Scalar stdDev;

        // calc stdDev inside window
        meanStdDev(window, mean,stdDev);
        float stdDevInWindow = stdDev[0];
        var.at<float>(0,j) = stdDevInWindow;

        varHist.append(stdDevInWindow);

        // TODO: heuristic
        bool largePeak = true;

        // calc mean in area behind our current pos.
        Mat prev = byteArray2Mat(echo.mid(j+wSize));
        meanStdDev(prev, mean, stdDev);
        float meanBehind = mean[0];

        // take first peak found.
        if (stdDevInWindow > stdDevInWindowTH && meanBehind<meanBehindTH && largePeak && K<0)
            K= j;

    }

    varHistory[data.dateTime] = varHist;

    return K;
}

Mat Module_SonarLocalization::byteArray2Mat(QByteArray array)
{
    Mat m = Mat::zeros(array.size(),1,CV_32F);
    for(int i=0;i<array.size();i++) {
        m.at<float>(0,i) = array[i];
    }
    return m;
}
