#include "sonarechofilter.h"

#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>
#include <Module_ScanningSonar/module_scanningsonar.h>

using namespace cv;

SonarEchoFilter::SonarEchoFilter(Module_ScanningSonar* sonar)
    :QObject()
{
    this->sonar = sonar;

    logger = Log4Qt::Logger::logger("SonarFilter");

    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), this, SLOT(newSonarData(SonarReturnData)));

    this->darknessCount = 0;
    this->swipedArea = 0;
    this->currentID = 0;

}


void SonarEchoFilter::newSonarData(SonarReturnData data)
{
    if (!sonar->isEnabled())
        return;

    QByteArray byteEcho = data.getEchoData();
    logger->trace("Received " + QString::number(byteEcho.size())+ " sonar echos.");

    // TODO: why 252??? it should be 250, or not?
    byteEcho.chop(byteEcho.size()-N);
    Mat echo = byteArray2Mat(byteEcho);

    echo = echo/127.0;

    // filter out noise etc.
    Mat echoFiltered = filterEcho(data,echo);

    rawHistory[data.dateTime] = mat2QVector(echo);

    filteredHistory[data.dateTime] = mat2QVector(echoFiltered);

    int K = findWall(data,echoFiltered);

    // pending later: SVM

    // pending later: Blob-Filter

    if (K<0) {
        darknessCount++;
    } else {
        logger->trace("found peak at "+QString::number(K));
        darknessCount=0;
        K_history.append(K);
        kHistory[data.dateTime]=K;

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

        swipedArea += abs(delta);
    }

    // todo: this must be absolutely robust!!
    if (localKlist.size()>0 && (darknessCount>=20 || swipedArea>350)) {
        // connect Ks until we have a closed image
        // transform from polar coordinates int euclid coordinates
        // form lines between points

        posArray.clear();
        for(int i=0; i<localKlist.size(); i++) {
            double x,y;
            // TODO: why +50 ?
            int range = 50;
            x = cos(localKlistHeading[i]/180*M_PI)*localKlist[i]/N*range;
            y = sin(localKlistHeading[i]/180*M_PI)*localKlist[i]/N*range;
//            X1=round(cosd(startHeading)*startDistance/P*r)+50;
//            Y1=round(sind(startHeading)*startDistance/P*r)+50;
//            X2=round(cosd(stopHeading)*stopDistance/P*r)+50;
//            Y2=round(sind(stopHeading)*stopDistance/P*r)+50;

//            if (i>0 && localKlistID[i]==localKlistID[i-1]+1) {
//                for (int k=localKlist[i-1]; k<=localKlist[i]; k++) {
//                    double ax = cos(localKlistHeading[i]/180*M_PI)*k/N*range;
//                    double ay = sin(localKlistHeading[i]/180*M_PI)*k/N*range;
//                    this->posArrayX.append(ax);
//                    this->posArrayY.append(ay);
//                }
//            }
//            this->posArray.append(QVector2D(x,y));
            if (sqrt(x*x+y*y)>10) // ahhhh: evil heuristic!
                this->posArray.append(QVector2D(x,y)); // TODO mirror then adaptively
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

int SonarEchoFilter::findWall(SonarReturnData data,const Mat& echo)
{
    // find last maximum

    int wSize=3;

    QVector<double> varHist(N);
    QVector<double> meanHist(N);

    int K = -1;

    //Mat var = Mat::zeros(1,N,CV_32F);
    for(int j=N-wSize-1; j>=wSize; j--) {

        // window around the point we're looking at
        Mat window = echo.colRange(j-wSize, j+wSize);

        Scalar mean;
        Scalar stdDev;

        // calc stdDev inside window
        meanStdDev(window, mean,stdDev);
        float stdDevInWindow = stdDev[0]*stdDev[0];
        //var.at<float>(0,j) = stdDevInWindow;

        varHist[j]=stdDevInWindow;

        // TODO: fiddle with TH, or move it a little bit to the left
        bool largePeak = mean[0]>0.5;


        // calc mean in area behind our current pos.
        Mat prev = echo.colRange(j+wSize,echo.cols-1);
        meanStdDev(prev, mean, stdDev);
        float meanBehind = mean[0];

        meanHist[j]=meanBehind;

        // take first peak found.
        if (stdDevInWindow > stdDevInWindowTH && meanBehind<meanBehindTH && largePeak && K<0) {
            logger->debug("stdDevInWindow="+QString::number(stdDevInWindow));
            K= j;
        }

    }

    varHistory[data.dateTime] = varHist;
    meanHistory[data.dateTime] = meanHist;

    return K;
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
