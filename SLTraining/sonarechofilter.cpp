#include "sonarechofilter.h"
#include <sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>

using namespace cv;

SonarEchoFilter::SonarEchoFilter()

//    : s(parent->getSettingsCopy())
{
//    this->sloc = parent;

    reset();

    gGaussFactor = 0.7870;
    gVarianceTH = 0.04;
    gWallWindowSize = 3;
    gLargePeakTH = 0.5;
    gMeanBehindTH = 1.0;
}

/**
  * do some preprocessing on newly received sonar data.
  * try to find the wall, and put the wall position in a queue.
  * if we found a sufficiently long consequtive piece of wall,
  * group them in an image and send it off to the particle filter.
  */
QByteArray SonarEchoFilter::newSonarData(SonarReturnData data)
{

    QByteArray byteEcho = data.getEchoData();
    
    if (byteEcho.size() != 252)
        qDebug() << "Received " << QString::number(N) << " sonar echos: Didn't test this!";

    // throw away the last two samples to make some calculations easier
    byteEcho.chop(byteEcho.size()-N);
    Mat echo = byteArray2Mat(byteEcho);

    // normalize
    echo /= MAX;

    // filter out noise etc.
    Mat echoFiltered = filterEcho(data,echo);

    rawHistory[data.switchCommand.time] = mat2QVector(echo);

    filteredHistory[data.switchCommand.time] = mat2QVector(echoFiltered);

//    int K = findWall(data,echoFiltered);

    QByteArray arr = mat2byteArray(echoFiltered);
    return arr;

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
        // changed gaus factor
        float gF = gGaussFactor;
        echoFiltered.at<float>(0,i) =  window.at<float>(0,0)*(1-gF)/2
                                     + window.at<float>(0,1)*gF
                                     + window.at<float>(0,2)*(1-gF)/2;

        //GAIN=16: sensorNoiseThresh=max((7/20)*((1:250)-50),0);
        // TODO: either adapt or settings!
        float cutOff=0;
        if (data.switchCommand.startGain==15)
            cutOff = (7.0/20)*(i-50)/127;
        else {
            //changed a1 times a2
            cutOff = (7.0/20)*(i-50)/127;
//            qDebug() << "Using parameters as gain.";
        }

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

void SonarEchoFilter::addToList(QList<QVector2D>& list, const QVector2D p)
{
    if (list.length()==0) {
        list.append(p);
        return;
    }

    QVector2D& q = list[list.length()-1];
    if ((q-p).length()>1)
        list.append(p);
}

int SonarEchoFilter::findWall(SonarReturnData data,const QByteArray echoBA)
{
    const cv::Mat echo = this->byteArray2Mat(echoBA);

    // find last maximum

    //changed wallwindows size
    int wSize= gWallWindowSize;

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
        //changed
        bool largePeak = mean[0]>gLargePeakTH;

        // calc mean in area behind our current pos.
        Mat prev = echo.colRange(j+wSize,echo.cols-1);
        meanStdDev(prev, mean, stdDev);
        float meanBehind = mean[0];

        meanHist[j]=meanBehind;

        // take first peak found.
        if (stdDevInWindow > gVarianceTH
            && meanBehind< gMeanBehindTH
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
    this->DEBUG = true;

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

QByteArray SonarEchoFilter::mat2byteArray(cv::Mat &mat)
{
    QByteArray arr;
    for(int i=0;i<mat.cols;i++)
    {
        arr.append(mat.at<float>(0,i));
    }
    for(int i=arr.size();i<252;i++)
        arr.append(0.0);
    return arr;
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
