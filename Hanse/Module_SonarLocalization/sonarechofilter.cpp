#include "sonarechofilter.h"
#include <Module_ScanningSonar/sonarreturndata.h>
#include <opencv/cv.h>
#include <QtCore>
#include <QtAlgorithms>
#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_XsensMTi/module_xsensmti.h>
#include <Module_Simulation/module_simulation.h>
#include <Framework/Angles.h>

using namespace cv;

SonarEchoFilter::SonarEchoFilter(Module_SonarLocalization* parent, Module_XsensMTi *mti, Module_Simulation *sim)
//    : s(parent->getSettingsCopy())
{
    this->sloc = parent;
    this->mti = mti;
    this->sim = sim;

    logger = Log4Qt::Logger::logger("SonarFilter");

    this->initNoiseMat();
    svm = new SVMClassifier();
    reset();
    this->lastMaxValue = -1;
    this->lastMaxValues.clear();

    temp_area = 0;

    lastValidDataHeading = 0;
    currentDataHeading = 0;
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

    SonarEchoData currData = SonarEchoData(data);

    if (this->sloc->getSettingsValue("use xsens").toBool()) {
        currData.setCompassHeading(mti->getHeading());
        currData.setHeadingIncrement(mti->getHeadingIncrement());
    }

    this->filterEcho(currData);
    if (this->sloc->getSettingsValue("medianFilter").toBool()) {
        this->medianFilter(currData);
    }

    this->gradientFilter(currData);
    int predClass = 1;
    currData.setClassLabel(predClass);
    candidates.append(currData);

    this->grouping();
}

void SonarEchoFilter::filterEcho(SonarEchoData &data)
{
    Mat echo = this->byteArray2Mat(data.getRawData());
    int N = echo.cols;

    // No filtering on simulator image.
    if (sim->isEnabled()) {
        data.setFiltered(this->mat2byteArray(echo));
    } else {
        Mat echoFiltered = Mat::zeros(1,N,CV_32F);

        // Estimate signal to noise ratio.
        int gain = data.getGain();
        for (int j = 0; j < N; j++) {
            if(gain < 17) {
                if (j < noiseMat.cols) {
                    echoFiltered.at<float>(0,j) = qMax(0.0f, echo.at<float>(0,j) - noiseMat.at<float>(gain-1,j));
                } else {
                    echoFiltered.at<float>(0,j) = qMax(0.0f, echo.at<float>(0,j));
                }
            } else {
                echoFiltered.at<float>(0,j) = 0.0;
            }
        }
        data.setFiltered(this->mat2byteArray(echoFiltered));
    }
}

void SonarEchoFilter::gradientFilter(SonarEchoData &data)
{
    Mat echoFiltered = this->byteArray2Mat(data.getFiltered());
    int N = echoFiltered.cols;

    if (sim->isEnabled()) {
        // In simulation, we have perfect data. A wall is indicated by
        // a value > 0.
        float maxVal = 0;
        int maxIdx = 0;
        for (int i = 0; i < N; i++) {
            if (echoFiltered.at<float>(0,i) > maxVal) {
                maxVal = echoFiltered.at<float>(0,i);
                maxIdx = i;
            }
        }

        if (maxVal > 20) {
            data.setWallCandidate(maxIdx);
        } else {
            data.setWallCandidate(-1);
        }
        data.setGradient(this->mat2List(echoFiltered));
    } else {
        // Do some more image processing.
        Mat integral = echoFiltered.clone();

        // Calculate integral image.
        for (int i = 1; i < N; i++) {
            integral.at<float>(0,i) = integral.at<float>(0,i) + integral.at<float>(0,i-1);
        }

        echoFiltered = Mat::ones(1, integral.cols, CV_32F);

        // Calculate gradient (and at the same time the maximum value,
        // which may be used as the wall candidate).
        QList<int> ks;
        ks.clear();
//        ks.append(24);
//        ks.append(12);
        ks.append(8);
        ks.append(4);
        ks.append(2);
        int k;
        foreach(k, ks) {
            for (int j = 0; j < N; j++) {
                int up = qMax(0, j - k);
                int down = qMin(j + k, N - 1);
                float gradient = 0;
                if (up == j - k && down == j + k) {
                    gradient = qMax(0.0f,(2 * integral.at<float>(0,j) - integral.at<float>(0,up) - integral.at<float>(0,down)) / (2*k));
                }
                echoFiltered.at<float>(0,j) = echoFiltered.at<float>(0,j) * gradient;
            }
        }

        data.setGradient(this->mat2List(echoFiltered));

        float maxValTH = this->sloc->getSettingsValue("gradientMaxVal",20).toFloat();
        int maxIdxTH = this->sloc->getSettingsValue("gradientMaxIdx",40).toInt();
        if (lastMaxValue > 0) {
            maxValTH = maxValTH*lastMaxValue;
        }

        // Non-Maximum Suppression: in a small window, ignore those values that are not maximum.
        QList<int> maximums;
        int K = 10;
        for (int i = K; i < N - K - 1; i++) {
            if (echoFiltered.at<float>(0,i) > maxValTH && echoFiltered.at<float>(0,i) > maxIdxTH) {
                bool isMaximum = true;
                for (int j = i - K; j <= i + K; j++) {
                    if ((echoFiltered.at<float>(0,j) >= echoFiltered.at<float>(0,i)) && (j != i)) {
                        isMaximum = false;
                        break;
                    }
                }
                if (isMaximum) {
                    maximums.append(i);
                }
            }
        }

        if (maximums.size() > 0) {
            data.setWallCandidate(maximums.last());
        } else {
            data.setWallCandidate(-1);
        }

//        // Estimate wall candidate as bin with maximum response.
//        float maxVal = 0;
//        float maxIdx = 0;
//        for (int i = 0; i < N; i++) {
//            if (echoFiltered.at<float>(0,i) > maxVal) {
//                maxVal = echoFiltered.at<float>(0,i);
//                maxIdx = i;
//            }
//        }

//        // Set as wall, if response is above some threshold (depending on
//        // maximum value in the last frames).
//        if (maxVal > maxValTH && maxIdx > maxIdxTH) {
//            data.setWallCandidate(maxIdx);
//        } else {
//            data.setWallCandidate(-1);
//        }
    }
}

void SonarEchoFilter::medianFilter(SonarEchoData &data)
{
    QByteArray echo = data.getFiltered();
    int w = 1;
    for (int i = 0; i < echo.size(); i++) {
        QByteArray window;
        for (int j = i - w; j <= i+w; j++) {
            if (j >= 0 && j < echo.size()) {
                window.append(echo[j]);
            }
        }
        qSort(window.begin(), window.end());
        echo[i] = window[(int)floor((window.size() - 1.0) / 2)];

        switch (i) {
        case 50:
            w = 1;
            break;
        case 100:
            w = 2;
            break;
        case 150:
            w = 3;
            break;
        case 200:
            w = 6;
            break;
        }
    }
    data.setFiltered(echo);
}

void SonarEchoFilter::findWall(SonarEchoData &data)
{
    Mat echo = this->byteArray2Mat(data.getFiltered());
    int N = echo.cols;
    int wSize= this->sloc->getSettingsValue("wallWindowSize").toInt();
    int K = -1;

    for(int j=N-wSize-1; j>=wSize; j--) {

        // window around the point we're looking at
        Mat window = echo.colRange(j-wSize, j+wSize);

        Scalar mean;
        Scalar stdDev;

        // calc stdDev inside window
        meanStdDev(window, mean,stdDev);
        float stdDevInWindow = stdDev[0]*stdDev[0];

        // TODO: fiddle with TH, or move it a little bit to the left
        float gLargePeakTH = this->sloc->getSettingsValue("largePeakTH").toFloat();
        bool largePeak = mean[0]>gLargePeakTH;

        // calc mean in area behind our current pos.
        Mat prev = echo.colRange(j+wSize,echo.cols-1);
        meanStdDev(prev, mean, stdDev);
        float meanBehind = mean[0];

        // take first peak found.
        float gVarianceTH = this->sloc->getSettingsValue("varTH").toFloat();
        float gMeanBehindTH = this->sloc->getSettingsValue("meanBehindTH").toFloat();
        if (stdDevInWindow > gVarianceTH
            && meanBehind< gMeanBehindTH
            && largePeak && K<0) {
            K=j;
            data.setWallCandidate(K);
            break;
        }
    }
}

void SonarEchoFilter::extractFeatures(SonarEchoData &data)
{
    if(data.getWallCandidate() == -1)
        return;

    Mat echo = this->byteArray2Mat(data.getFiltered());

    int xw = data.getWallCandidate();
     int kp = this->sloc->getSettingsValue("wallWindowSize").toInt();
     float f[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
     int xp = xw;
     for(int i=xw-kp; i<xw+kp;i++)
     {
         if(echo.at<float>(0,i) > f[0])
         {
             f[0] = echo.at<float>(0,i);
             xp = i;
         }
     }

     data.addFeature(0,f[0]);

     Scalar mean1 = Scalar();
     Scalar mean2 = Scalar();
     Scalar stdDev1 = Scalar();
     Scalar stdDev2 = Scalar();

     meanStdDev(echo.colRange(0,xw),mean1,stdDev1);
     meanStdDev(echo.colRange(xw+1,echo.cols-1),mean2,stdDev2);
     Scalar var1 = stdDev1.mul(stdDev1);
     Scalar var2 = stdDev2.mul(stdDev2);

     data.addFeature(1,mean1.val[0]/mean2.val[0]);
     data.addFeature(2,var1.val[0]/var2.val[0]);
     data.addFeature(3,xw);
     data.addFeature(4,xw -prevWallCandidate);
     f[1] = mean1.val[0]/mean2.val[0];
     f[2] = var1.val[0]/var2.val[0];
     f[3] = xw;
     f[4] = xw - prevWallCandidate;
     prevWallCandidate = xw;

     meanStdDev(echo.colRange(xp-kp,xp+kp),mean1,stdDev1);
     data.addFeature(5,mean1.val[0]);
     data.addFeature(6,stdDev1.val[0] * stdDev1.val[0]);
     f[5] = mean1.val[0];
     f[6] = stdDev1.val[0] * stdDev1.val[0];

     meanStdDev(echo.colRange(xw-kp,xw+kp),mean1,stdDev1);
     f[7] = mean1.val[0];
     f[8] = stdDev1.val[0] * stdDev1.val[0];
     data.addFeature(7,mean1.val[0]);
     data.addFeature(8,stdDev1.val[0] * stdDev1.val[0]);

}

void SonarEchoFilter::applyHeuristic()
{
    int deltaTH = this->sloc->getSettingsValue("darknessCnt").toInt();
    bool singlePoint = this->sloc->getSettingsValue("singlePoint").toBool();
    bool deltaK = this->sloc->getSettingsValue("deltaKH").toBool();

    //DeltaKMethod
    if(deltaK)
    {
        for(int i = 1; i < candidates.size(); i++)
        {
            int wcPrev = candidates[i-1].getWallCandidate();
            int wcCurr = candidates[i].getWallCandidate();
            int absolut = abs(wcPrev-wcCurr);
            if((absolut > deltaTH) && (wcPrev > 1))
            {
//                qDebug() << i << " wc " <<wcPrev << wcCurr;
                candidates[i].setWallCandidate(-1);
                candidates[i].setClassLabel(0);
            }
        }
    }

    //SinglePoint Method
    if(singlePoint)
    {
        for(int i = 1; i < candidates.size()-1; i++)
        {
            bool prev = (candidates[i-1].getWallCandidate() > 1);//candidates[i-1].hasWallCandidate();
            bool next = (candidates[i+1].getWallCandidate() > 1);//candidates[i+1].hasWallCandidate();
            prev = (candidates[i-1].getClassLabel() == 1);
            next = (candidates[i+1].getClassLabel() == 1);

            if(!prev && !next)
            {
                candidates[i].setWallCandidate(-1);
                candidates[i].setClassLabel(0);
            }
        }
    }
}

void SonarEchoFilter::grouping()
{
    int maxCutTH = this->sloc->getSettingsValue("groupingMaxArea",360).toInt();
    float diff = 0;

    if (candidates.size() > 1) {
        int i = candidates.size() - 1;

        if (this->sloc->getSettingsValue("use xsens").toBool()) {
            diff = (candidates[i-1].getHeadPosition() + candidates[i-1].getHeadingIncrement()) -
                   (candidates[i].getHeadPosition() + candidates[i].getHeadingIncrement());
        } else {
            diff = candidates[i-1].getHeadPosition() - candidates[i].getHeadPosition();
        }

        diff = Angles::deg2deg(diff);

        newDirection = candidates[i-1].getHeadPosition() - candidates[i].getHeadPosition();

        if(newDirection > 0)
            newDirection = 1;
        else if(newDirection < 0)
            newDirection = -1;
        else
            newDirection= 0;
    } else {
        newDirection = 1;
        diff = 0;
    }

    // Add heading difference to total sonar head movement.
    temp_area += abs(diff);
//    mti->addData("area", temp_area);
//    qDebug("%f", temp_area);

    // Do grouping, if enough data was collected.
    if (temp_area > maxCutTH) {

        // Find cutting index.
        int cutIndex = 0;
        int darknessCount = this->sloc->getSettingsValue("groupingDarkness").toInt();
        for (int i = candidates.size()-1; i > 0; i--) {
            if (candidates[i].getWallCandidate() > 1 && candidates[i].getClassLabel() == 1)
                darknessCount = this->sloc->getSettingsValue("groupingDarkness").toInt();
            else
                darknessCount--;

            if (darknessCount == 0) {
                cutIndex = i;
                break;
            }
        }

        currentDataHeading = candidates.last().getCompassHeading();
        sloc->addData("heading at current sonar reading", currentDataHeading);

        if (cutIndex == 0) {
            qDebug() << "No Darkness. Cutting Candidates at 360 degrees";
            this->sendImage();
        } else {
            qDebug() << "cut at " << cutIndex << " of " << candidates.size();
            QList<SonarEchoData> tmp;

            temp_area = 0;
            diff = 0;

            //splitting candidates at cutIndex
            int total = candidates.size();
            for(int i = cutIndex; i < total; i++)
            {
                int last = candidates.size()-1;

                if (this->sloc->getSettingsValue("use xsens").toBool()) {
                    diff = (candidates[last-1].getHeadPosition() + candidates[last-1].getHeadingIncrement()) -
                           (candidates[last].getHeadPosition() + candidates[last].getHeadingIncrement());
                } else {
                    diff = candidates[last-1].getHeadPosition() - candidates[last].getHeadPosition();
                }

                diff = Angles::deg2deg(diff);
                temp_area += abs(diff);

                //just keep positiv wallCandidates
//                if (candidates.last().getClassLabel() == 1)
                    tmp.append(candidates.takeLast());
//                else
//                    candidates.removeLast();

            }
            this->sendImage();
            candidates.clear();

            //restore candidates
//            for (int i = 0; i < tmp.size(); i++)
//                candidates.append(tmp.takeFirst());
            candidates = tmp;
        }
    }
}

void SonarEchoFilter::sendImage()
{
    this->applyHeuristic();
    QList<QVector2D> posArray;
    posArray.clear();
    QList<SonarEchoData> wallFeatures;

    // Try to find the ground echo.
    float maxValue = 0;
    float groundIdx = 0;
    for (int i = 0; i < 50; i++) {
        float mean = 0;
        for (int j = 0; j < candidates.size(); j++) {
//            QList<float> data = candidates[j].getGradient();
            QByteArray data = candidates[j].getFiltered();
            mean += (float)data[i];
        }
        mean /= candidates.size();
        if (maxValue < mean) {
            maxValue = mean;
            groundIdx = i;
        }
    }

    if (!sim->isEnabled()) {
        if (maxValue > 0.001*lastMaxValue) {
            // Filter those wall candidates that might be ground.
            for (int j = 0; j < candidates.size(); j++) {
                int wc = candidates[j].getWallCandidate();
                if (qAbs(wc - groundIdx) <= 10) {
                    candidates[j].setWallCandidate(-1);
                }
            }
        }
    }

    // Set heading increments for correction of the sonar values. We do this
    // from back to front to obtain a correction relative to the most recent
    // sonar reading.
    float accumulatedHeadingIncrement = 0;
    if (this->sloc->getSettingsValue("use xsens").toBool()) {
        for (int i = candidates.size() - 1; i >= 0; i--) {
            float headingIncrement = candidates[i].getHeadingIncrement();
            candidates[i].setHeadingIncrement(accumulatedHeadingIncrement);
            accumulatedHeadingIncrement -= headingIncrement;
        }
    }

    // Get Euclidian representation of wall features.
    for (int i = 0; i < candidates.size(); i++) {
        if (candidates[i].getWallCandidate() > 0 && candidates[i].getClassLabel() == 1) {
            QVector2D vec  = candidates[i].getEuclidean();
            posArray.append(vec);
            wallFeatures.append(candidates[i]);
        }
    }

    if (wallFeatures.size() > 0) {
        lastValidDataHeading = wallFeatures.last().getCompassHeading();
        sloc->addData("heading at last observation", lastValidDataHeading);
    }

    // Calculate maximum value in the current frame.
    lastMaxValue = 0.0f;
    for(int j = 0; j < wallFeatures.size(); j++) {
        QList<float> data = wallFeatures[j].getGradient();
        for (int i = 0; i < data.size(); i++) {
            if (data[i] > lastMaxValue) {
                lastMaxValue = data[i];
            }

        }
    }

    // Store a history of maximum values.
    lastMaxValues.append(lastMaxValue);
    while(lastMaxValues.size() < this->sloc->getSettingsValue("histMaxVal").toInt()+1) {
        lastMaxValues.append(lastMaxValue);
    }
    lastMaxValues.pop_front();

    // Compute lastMaxValue as maximum of lastMaxValues
    float maxi = 0.0f;
    for (int i = 0; i < lastMaxValues.size(); i++) {
        if (lastMaxValues[i] > maxi) {
            maxi = lastMaxValues[i];
        }
    }
    lastMaxValue = maxi;

    logger->debug("New Image");
    emit newImage(posArray);
    emit newSonarEchoData(wallFeatures);
    emit newSonarPlotData(candidates);

    //reset local variables
    candidates.clear();
    groupID++;
    temp_area = 0;
    newDirection = 0;
}

void SonarEchoFilter::getNoNoiseFilter(QVector<int> &vec)
{
    vec.clear();
    for(int i=0;i < noiseMat.rows; i++)
    {
        float sum = 0.0;
        for(int j =0; j<noiseMat.cols; j++)
        {
            sum+=noiseMat.at<float>(i,j);
        }
        if(sum > 0.0)
            vec.append(1);
        else
            vec.append(0);
    }
    if(vec.size() != noiseMat.rows)
        qDebug("ERROR Size of noiseMat != Size of NoNoiseFilter");
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

void SonarEchoFilter::reset()
{
    if(this->sloc)
        this->DEBUG = sloc->getSettingsValue("DEBUG").toBool();

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

    temp_area = 0;
    newDirection = 0;
    groupID = 1;
    prevWallCandidate = 0.0;
    candidates.clear();

//    QString path = this->sloc->getSettingsValue("Path2SVM").toString();
//    if(path.length() > 0)
//    {
//        QByteArray ba = path.toLatin1();
//          const char *c_str2 = ba.data();
//          svm->loadClassifier(c_str2);

//    }
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

QList<float> SonarEchoFilter::mat2List(cv::Mat &mat)
{
    QList<float> arr;
    for(int i=0;i<mat.cols;i++)
    {
        arr.append(mat.at<float>(0,i));
    }
//    for(int i=arr.size();i<250;i++)
//        arr.append(0.0);
    return arr;
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

float SonarEchoFilter::getLastObservationHeading()
{
    return lastValidDataHeading;
}

void SonarEchoFilter::initNoiseMat()
{
    float a[] = { 4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,4.89898989898990,5.11111111111111,5.51851851851852,4.40740740740741,5.11111111111111,4.92592592592593,4.11111111111111,4.62962962962963,4.85185185185185,4.81481481481482,5.07407407407407,4.92592592592593,6.29629629629630,6.18518518518519,6.85185185185185,7.55555555555556,7.66666666666667,8.29629629629630,6.92592592592593,8.29629629629630,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5.22222222222222,5.18518518518519,5.48148148148148,5.25925925925926,5.29629629629630,5.14814814814815,5.44444444444445,5.55555555555556,5.33333333333333,5.29629629629630,5.85185185185185,5.92592592592593,5.51851851851852,5.62962962962963,5.48148148148148,6.44444444444445,5.77777777777778,6.29629629629630,5.74074074074074,6.85185185185185,7.18518518518519,6.92592592592593,7.18518518518519,6.33333333333333,6.51851851851852,6,7.18518518518519,6.81481481481482,6.96296296296296,7.62962962962963,6.96296296296296,7.22222222222222,6.88888888888889,7.62962962962963,7.51851851851852,6.85185185185185,6.44444444444445,6.37037037037037,7.03703703703704,7.29629629629630,7.92592592592593,8.96296296296296,9.33333333333333,8.33333333333333,8.59259259259259,9.40740740740741,8.85185185185185,8.88888888888889,7.74074074074074,9.14814814814815,9.74074074074074,10.7777777777778,10.2962962962963,9.37037037037037,10.1481481481481,10.0370370370370,9.70370370370370,10.3703703703704,9.18518518518519,8.70370370370370,10.4444444444444,10.9629629629630,9.22222222222222,10.1481481481481,11.2962962962963,17.6666666666667,12.1481481481481,10.1851851851852,9.88888888888889,9.74074074074074,8.66666666666667,10.4074074074074,10.5185185185185,10.5185185185185,10.4444444444444,10.6666666666667,10.8518518518519,10.6296296296296,11.3333333333333,9.51851851851852,10,9.66666666666667,9.92592592592593,11.5925925925926,11.2592592592593,10.8148148148148,11.1111111111111,10.4814814814815,10.8148148148148,10.5925925925926,11.5925925925926,12.1111111111111,11.1481481481481,10.4444444444444,11.0370370370370,12.5555555555556,11.7407407407407,12.1481481481481,11.4074074074074,11.7777777777778,11.7777777777778,11.6666666666667,12.3333333333333,11.8888888888889,13.0740740740741,12.4074074074074,12.6296296296296,14.1851851851852,14.0370370370370,13.5555555555556,13.0370370370370,14.1111111111111,14.4074074074074,13.9629629629630,13.0370370370370,13.9629629629630,14.5925925925926,16.7037037037037,13.2222222222222,14.5925925925926,16.3703703703704,15.2592592592593,15.7777777777778,15.7777777777778,15.3703703703704,16.4074074074074,15.1851851851852,15.8888888888889,15.7037037037037,15.5925925925926,16.2962962962963,16.0740740740741,14.6666666666667,17.3703703703704,18.0370370370370,16.8518518518519,17.9259259259259,19.9629629629630,19.2592592592593,
                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                  2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.01893939393939,2.18750000000000,2.22916666666667,2.12500000000000,2.04166666666667,2.06250000000000,1.97916666666667,2.14583333333333,2.56250000000000,2.64583333333333,2.66666666666667,2.58333333333333,2.54166666666667,2.41666666666667,2.58333333333333,2.77083333333333,2.97916666666667,2.79166666666667,2.66666666666667,2.89583333333333,2.68750000000000,2.89583333333333,3.54166666666667,3.29166666666667,3.31250000000000,2.85416666666667,3.08333333333333,3.12500000000000,3.12500000000000,2.97916666666667,3.41666666666667,3.37500000000000,3.50000000000000,3.41666666666667,3.56250000000000,3.72916666666667,3.60416666666667,3.31250000000000,3.72916666666667,3.91666666666667,3.93750000000000,4.20833333333333,4.04166666666667,4.14583333333333,4.20833333333333,4.14583333333333,3.66666666666667,4.41666666666667,4.54166666666667,4.75000000000000,4.58333333333333,4.87500000000000,4.50000000000000,4.70833333333333,4.72916666666667,4.54166666666667,5.60416666666667,5.25000000000000,5.16666666666667,5.35416666666667,5.33333333333333,5.27083333333333,5.06250000000000,5.08333333333333,4.62500000000000,5.14583333333333,5.97916666666667,5.54166666666667,6.08333333333333,6.25000000000000,5.83333333333333,5.81250000000000,5.62500000000000,5.91666666666667,5.95833333333333,6.31250000000000,7.14583333333333,6.85416666666667,6.85416666666667,6.79166666666667,6.35416666666667,6.75000000000000,6.68750000000000,6.47916666666667,6.83333333333333,6.60416666666667,7.02083333333333,7.60416666666667,7.58333333333333,7.52083333333333,7.68750000000000,7.95833333333333,7.39583333333333,7.56250000000000,7.25000000000000,8.70833333333333,7.20833333333333,8.14583333333333,7.85416666666667,8.10416666666667,8.79166666666667,8.25000000000000,8.29166666666667,8.18750000000000,8.72916666666667,8.41666666666667,8.29166666666667,9.31250000000000,8.77083333333333,9.22916666666667,9.43750000000000,10.1250000000000,9.72916666666667,9.08333333333333,9.54166666666667,9.66666666666667,9.14583333333333,9.95833333333333,9.89583333333333,9.64583333333333,10.6875000000000,10.7708333333333,10.4791666666667,10.9583333333333,10.8333333333333,10.7708333333333,10.7291666666667,10.5625000000000,10.6666666666667,11.0833333333333,11,11.2916666666667,12.4166666666667,12.2083333333333,12.4791666666667,12.1875000000000,12.0625000000000,12.1250000000000,12.6458333333333,11.5000000000000,13.0833333333333,12.3958333333333,13.1041666666667,12.7708333333333,13.2708333333333,14.1041666666667,14.2083333333333,13.9166666666667,13.2708333333333,13.0625000000000,13.7916666666667,13,15.4166666666667,14.1666666666667,14.7083333333333,13.1041666666667,13.5416666666667,16.2708333333333,15.6875000000000,16.7083333333333,15.6250000000000,15.3541666666667,15.3333333333333,14.5000000000000,15.8541666666667,15.4583333333333,15.7708333333333,14.0833333333333,15.2083333333333,15.8333333333333,16.9375000000000,17.3958333333333,17.2916666666667,14.9583333333333,16.5000000000000,17.8958333333333,
                  4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,4.80980861244019,5.03947368421053,5.30263157894737,4.82894736842105,5.03947368421053,5.23684210526316,4.88157894736842,5.71052631578947,5.63157894736842,5.57894736842105,5.51315789473684,5.32894736842105,5.23684210526316,5.43421052631579,5.61842105263158,6.17105263157895,6.22368421052632,6.05263157894737,6.21052631578947,6.25000000000000,5.71052631578947,5.93421052631579,6.75000000000000,6.68421052631579,6.76315789473684,6.65789473684211,6.52631578947368,6.44736842105263,6.75000000000000,6.36842105263158,7.60526315789474,7.36842105263158,7.30263157894737,7.43421052631579,7.10526315789474,7.42105263157895,7.46052631578947,7.28947368421053,7.73684210526316,8.14473684210526,8.43421052631579,8.38157894736842,8.42105263157895,8.39473684210526,8.06578947368421,7.98684210526316,8.46052631578947,9.07894736842105,9.40789473684210,9.05263157894737,9.50000000000000,9.55263157894737,8.97368421052632,9.42105263157895,9.27631578947369,9.15789473684210,10.4736842105263,10.8815789473684,10.1578947368421,10.6973684210526,10.1710526315789,10.5394736842105,10.4736842105263,9.94736842105263,10.0526315789474,10.4736842105263,12.0921052631579,11.7763157894737,11.5657894736842,11.5394736842105,11.1710526315789,11.9605263157895,11.8684210526316,11.2631578947368,11.2894736842105,11.5263157894737,12.9473684210526,13.3947368421053,12.7763157894737,12.9868421052632,12.3947368421053,13.5789473684211,13.1315789473684,13.0657894736842,13.4078947368421,13.1578947368421,13.6710526315789,15.5000000000000,14.0921052631579,14.4605263157895,14.7631578947368,15.0789473684211,14.6842105263158,15.2894736842105,15.0526315789474,14.0526315789474,15.2236842105263,16.1447368421053,16.6184210526316,16.7894736842105,16.3815789473684,16.2763157894737,16.8026315789474,16.8157894736842,16.6578947368421,17.6973684210526,17.2368421052632,16.1578947368421,17.5000000000000,18.4736842105263,19.1578947368421,18.6842105263158,19.0789473684211,17.1184210526316,19.0131578947368,18.7236842105263,18.4605263157895,18.0921052631579,18.7368421052632,19.2631578947368,22,21.1578947368421,20.0789473684211,20.7368421052632,21.9868421052632,21.5131578947368,21.8552631578947,21.2763157894737,19.7631578947368,22.1973684210526,21.1315789473684,21.0131578947368,23,24.7894736842105,24.0263157894737,24.6842105263158,24.2631578947368,24.2894736842105,23.7894736842105,24.1315789473684,24.2236842105263,24.8421052631579,23.2105263157895,23.2236842105263,25.1842105263158,26.1184210526316,28.4342105263158,27.3157894736842,26.5657894736842,26.7500000000000,26.9342105263158,25.4868421052632,28.7500000000000,25.6052631578947,27.3157894736842,27.1052631578947,28.0526315789474,29.6184210526316,30.9078947368421,29.4605263157895,30.2500000000000,30.1578947368421,30.9736842105263,30.5789473684211,30,31.1315789473684,30.6052631578947,31.0394736842105,30.0394736842105,29.3026315789474,33.1184210526316,34.5131578947368,34.3947368421053,35.1447368421053,33.4736842105263,34.7763157894737,
                  4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.39935064935065,4.78571428571429,4.32142857142857,3.92857142857143,4.21428571428571,4.71428571428571,4.89285714285714,4.75000000000000,5.39285714285714,5.46428571428571,4.96428571428571,5.39285714285714,4.64285714285714,4.82142857142857,5.57142857142857,4.89285714285714,5.67857142857143,5.14285714285714,5.46428571428571,5.67857142857143,5.46428571428571,5.42857142857143,5.96428571428571,5.67857142857143,6.32142857142857,6,6.78571428571429,5.57142857142857,5.89285714285714,5.82142857142857,6.96428571428571,7.17857142857143,6.96428571428571,6.78571428571429,6.82142857142857,7,6.42857142857143,7.28571428571429,6.92857142857143,7.82142857142857,7.82142857142857,8.07142857142857,7.14285714285714,7.64285714285714,7.53571428571429,7.82142857142857,7.89285714285714,8.53571428571429,8.64285714285714,8.14285714285714,8.64285714285714,9.03571428571429,9.17857142857143,8.17857142857143,8.89285714285714,8.96428571428571,9.14285714285714,10,10.1071428571429,9.32142857142857,9.75000000000000,9.85714285714286,9.64285714285714,9.75000000000000,8.67857142857143,10.0357142857143,11.1071428571429,11.2500000000000,11.0714285714286,11.1071428571429,9.78571428571429,10.0357142857143,10.4285714285714,10.8214285714286,9.53571428571429,10,11.6785714285714,11.5714285714286,12.5000000000000,11.7500000000000,12.2500000000000,12.2857142857143,12.2142857142857,11.9642857142857,11.5714285714286,12.0714285714286,13.2857142857143,13.9285714285714,13.8928571428571,12.7857142857143,12.3214285714286,13.1785714285714,13.4642857142857,13.5000000000000,12.9642857142857,13.0714285714286,13.7500000000000,14.9642857142857,14.2857142857143,15.2857142857143,15.1071428571429,14.1071428571429,15.2500000000000,15.4285714285714,14.6071428571429,16.5000000000000,15.6428571428571,14.8928571428571,16.5714285714286,16.2500000000000,17.6785714285714,16.8571428571429,17.4642857142857,17.0357142857143,17,17.3571428571429,16.2857142857143,16.2500000000000,15.9642857142857,17.1428571428571,18.6428571428571,19.8571428571429,18.2142857142857,19.2500000000000,17.8571428571429,19.1071428571429,18.2142857142857,19.6428571428571,21.4642857142857,16.3214285714286,20.1428571428571,17.7142857142857,21.1071428571429,20.1428571428571,20.8928571428571,21.1785714285714,21.5000000000000,21.0357142857143,22.3928571428571,20.7142857142857,23.9285714285714,20.1071428571429,23.1785714285714,20.3571428571429,22.3214285714286,24,23.4642857142857,23.3214285714286,23.5000000000000,23.5714285714286,22,23.0714285714286,22.3571428571429,24.2500000000000,22.2142857142857,24.0714285714286,23.8928571428571,25.7500000000000,25.9642857142857,26.3928571428571,28.5000000000000,25.7142857142857,27.7857142857143,27.6428571428571,25.5357142857143,25.5000000000000,27.3214285714286,25.4642857142857,26.8571428571429,25.4285714285714,27.1785714285714,29.3571428571429,28.2142857142857,29.8214285714286,29.5000000000000,29,
                  3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.44055944055944,3.53846153846154,3.92307692307692,3.46153846153846,3.69230769230769,3.38461538461538,3.61538461538462,3,3.23076923076923,4,4.46153846153846,3.61538461538462,4.38461538461539,3.30769230769231,3.84615384615385,4.23076923076923,4.15384615384615,4.46153846153846,4,4.30769230769231,3.53846153846154,4.07692307692308,4.84615384615385,4.30769230769231,4.84615384615385,4.15384615384615,4.07692307692308,4.92307692307692,4.15384615384615,4.69230769230769,5.46153846153846,5.15384615384615,5.23076923076923,5.76923076923077,5.15384615384615,4.53846153846154,5.15384615384615,4.46153846153846,5.46153846153846,5.92307692307692,4.92307692307692,6.23076923076923,5.23076923076923,5.46153846153846,6.07692307692308,5.53846153846154,6.76923076923077,7,6.23076923076923,6.76923076923077,6.76923076923077,6.23076923076923,6.92307692307692,7,6.76923076923077,6.15384615384615,7.46153846153846,7.23076923076923,7.69230769230769,7.76923076923077,8.46153846153846,6.61538461538462,7.53846153846154,8.07692307692308,8.61538461538462,8.76923076923077,8.38461538461539,7.76923076923077,8.92307692307692,7.92307692307692,7.76923076923077,8.07692307692308,8.23076923076923,8.84615384615385,8.76923076923077,7.38461538461539,9.30769230769231,9.53846153846154,8.84615384615385,8.69230769230769,8.23076923076923,9.84615384615385,9.92307692307692,9.15384615384615,10,11.3076923076923,8.92307692307692,10.8461538461538,10.7692307692308,11.0769230769231,9.61538461538462,11.6923076923077,10.3846153846154,10.3076923076923,10.5384615384615,10,12.1538461538462,12.6153846153846,12,12,11.6153846153846,11.7692307692308,11.6923076923077,13.2307692307692,11.9230769230769,12.0769230769231,12.7692307692308,12.3846153846154,12.6923076923077,12.3076923076923,15.6153846153846,12.7692307692308,13.9230769230769,14.3846153846154,12.7692307692308,13.1538461538462,12.8461538461538,14.1538461538462,13.5384615384615,13.6923076923077,14.3076923076923,15.7692307692308,16.3846153846154,14.9230769230769,16.0769230769231,15.4615384615385,13.3076923076923,15.6153846153846,15.7692307692308,16.3846153846154,15.6923076923077,15.6923076923077,17,17,17.1538461538462,15.6153846153846,18.2307692307692,16.4615384615385,17.3846153846154,19.3846153846154,17.4615384615385,15.4615384615385,15.7692307692308,17.2307692307692,17.1538461538462,18.9230769230769,20.0769230769231,19.3846153846154,19.1538461538462,18.3076923076923,17.6923076923077,18.4615384615385,17.6923076923077,23.8461538461538,17.9230769230769,21.5384615384615,21.8461538461538,19.7692307692308,22.2307692307692,22.6923076923077,20.3076923076923,21.6153846153846,22.3076923076923,19.6923076923077,20.3846153846154,21.4615384615385,22.7692307692308,22.2307692307692,23.9230769230769,18.4615384615385,23.6923076923077,26.7692307692308,26.3846153846154,25.9230769230769,23.6153846153846,25.5384615384615,
                  3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.38593481989708,3.86792452830189,3.39622641509434,3.43396226415094,3.75471698113208,3.58490566037736,3.50943396226415,3.50943396226415,3.90566037735849,4.09433962264151,4.15094339622642,3.67924528301887,3.96226415094340,3.81132075471698,3.92452830188679,4.37735849056604,4.83018867924528,4.66037735849057,4.77358490566038,4.35849056603774,4.56603773584906,4.79245283018868,5.24528301886793,5.41509433962264,4.98113207547170,5.11320754716981,5.05660377358491,5.30188679245283,5.11320754716981,4.92452830188679,5.58490566037736,5.77358490566038,6.15094339622642,6.33962264150943,5.35849056603774,5.37735849056604,5.84905660377359,5.37735849056604,6.30188679245283,6.75471698113208,7.13207547169811,6.49056603773585,6.62264150943396,6.79245283018868,6.35849056603774,6.71698113207547,6.81132075471698,7.56603773584906,7.39622641509434,7.52830188679245,7.54716981132076,7.58490566037736,6.84905660377359,7.28301886792453,6.88679245283019,7.35849056603774,8.07547169811321,7.83018867924528,8.83018867924528,8.75471698113208,8.45283018867925,8.77358490566038,8.32075471698113,8.98113207547170,7.98113207547170,8.22641509433962,9.45283018867925,9.22641509433962,9.32075471698113,9.39622641509434,9.69811320754717,10.0943396226415,8.94339622641510,8.98113207547170,9.62264150943396,9.13207547169811,11.0566037735849,9.96226415094340,10.6603773584906,10.3207547169811,10.9245283018868,10.8867924528302,10.1132075471698,10.9433962264151,10.2264150943396,10.2075471698113,11.1698113207547,12.2830188679245,11.9245283018868,11.8113207547170,12.0566037735849,11.4905660377359,12.3018867924528,10.8113207547170,11.8301886792453,11.2830188679245,12.5471698113208,12.9056603773585,13.7358490566038,13.0188679245283,14.6981132075472,13.2264150943396,13.4339622641509,12.8679245283019,13.0377358490566,14.2830188679245,13.8867924528302,13.1509433962264,14.8490566037736,15.6037735849057,15.4150943396226,14.9433962264151,14.3773584905660,14.6226415094340,14.7358490566038,14.2075471698113,15.4905660377359,15.1509433962264,14.9811320754717,14.6415094339623,17.0566037735849,16.1886792452830,15.5471698113208,16.9433962264151,18.5660377358491,16.4905660377359,17.1320754716981,17.0754716981132,17.9622641509434,16.6037735849057,17.1132075471698,16.3207547169811,18.1509433962264,20.5660377358491,19.1698113207547,20.5471698113208,19.5849056603774,18.4716981132075,19.5471698113208,21.7169811320755,20.4528301886792,19.8301886792453,20.4339622641509,18.8490566037736,20.9622641509434,22.6603773584906,23.0188679245283,21.8301886792453,22.5849056603774,21.5471698113208,22.7547169811321,20.4905660377359,22.0566037735849,22.5283018867925,21.2264150943396,22.2641509433962,21.3207547169811,24.8301886792453,24.7169811320755,25.3584905660377,24.2830188679245,24.5283018867925,24.6981132075472,22.5849056603774,25.3018867924528,24.7169811320755,23.6226415094340,23.5660377358491,25.2075471698113,24.5094339622642,26.5094339622642,28.6792452830189,28.4716981132075,27.8490566037736,27.7169811320755,26.7169811320755,
                  4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,3.88636363636364,4.22727272727273,4.27272727272727,4.29545454545455,3.86363636363636,4.25000000000000,4.38636363636364,4.54545454545455,4.84090909090909,4.36363636363636,4.29545454545455,5.11363636363636,4.52272727272727,4.93181818181818,4.70454545454545,4.90909090909091,5.54545454545455,5.09090909090909,5.20454545454545,5.11363636363636,4.52272727272727,5.04545454545455,5.63636363636364,6.40909090909091,5.27272727272727,5.81818181818182,5.43181818181818,5.72727272727273,5.61363636363636,6.34090909090909,6.45454545454545,6.25000000000000,6.54545454545455,7.06818181818182,6.54545454545455,6.09090909090909,6.06818181818182,6.70454545454545,7.52272727272727,7.50000000000000,7.27272727272727,7.04545454545455,7.11363636363636,7.31818181818182,7.27272727272727,7.02272727272727,8.22727272727273,7.88636363636364,8.34090909090909,7.72727272727273,7.90909090909091,7.84090909090909,8.56818181818182,8.22727272727273,8.13636363636364,8.18181818181818,8.88636363636364,9.65909090909091,9.34090909090909,9.50000000000000,9.11363636363636,9.20454545454546,9.27272727272727,9.04545454545455,9.29545454545455,10.4090909090909,10.0454545454545,10.2954545454545,11.0681818181818,10.3636363636364,11.0681818181818,10.5000000000000,10.2500000000000,10.1363636363636,9.52272727272727,11.4090909090909,11.4318181818182,11.5681818181818,11.9772727272727,12.2272727272727,12.5227272727273,12.1590909090909,12.4545454545455,11.3863636363636,12.0681818181818,13.5454545454545,13.1136363636364,11.8181818181818,13.3863636363636,13.2045454545455,13.8636363636364,12.8409090909091,13.0454545454545,13.2045454545455,13.1363636363636,13.3181818181818,13.8409090909091,14.7954545454545,15.3409090909091,14.3636363636364,15.8409090909091,14.5000000000000,14.9545454545455,14.2500000000000,16.2727272727273,14.6136363636364,14.6136363636364,15.6363636363636,17.3181818181818,17.0454545454545,16.4545454545455,16.3636363636364,16.9318181818182,16.3181818181818,17.3863636363636,16.9545454545455,17.2045454545455,15.7272727272727,16.0681818181818,18.0909090909091,18.0681818181818,20.0227272727273,19.8181818181818,17.7954545454545,17.7954545454545,18.9318181818182,18.7954545454545,19.2954545454545,18.6363636363636,18.4090909090909,19.5681818181818,22.4318181818182,21.5454545454545,21.2727272727273,21.5909090909091,20.8636363636364,21.0227272727273,20.7272727272727,22.4090909090909,21.0909090909091,21.1590909090909,20.7727272727273,20.2954545454545,24.5681818181818,23.9090909090909,24.4772727272727,24.2045454545455,25.0454545454545,23,23.6363636363636,23.6818181818182,22.5909090909091,24.6590909090909,24.9318181818182,24.6363636363636,22.5681818181818,27.3863636363636,26.5454545454545,25.9772727272727,23.9545454545455,26.7272727272727,27.8181818181818,26.9318181818182,29.3409090909091,27.2045454545455,25.8181818181818,25.9090909090909,26.5227272727273,27.6136363636364,27.4545454545455,27.4090909090909,30.8636363636364,27.8863636363636,32.0681818181818,28.8863636363636,
//                  4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,4.03305785123967,3.88636363636364,4.22727272727273,4.27272727272727,4.29545454545455,3.86363636363636,4.25000000000000,4.38636363636364,4.54545454545455,4.84090909090909,4.36363636363636,4.29545454545455,5.11363636363636,4.52272727272727,4.93181818181818,4.70454545454545,4.90909090909091,5.54545454545455,5.09090909090909,5.20454545454545,5.11363636363636,4.52272727272727,5.04545454545455,5.63636363636364,6.40909090909091,5.27272727272727,5.81818181818182,5.43181818181818,5.72727272727273,5.61363636363636,6.34090909090909,6.45454545454545,6.25000000000000,6.54545454545455,7.06818181818182,6.54545454545455,6.09090909090909,6.06818181818182,6.70454545454545,7.52272727272727,7.50000000000000,7.27272727272727,7.04545454545455,7.11363636363636,7.31818181818182,7.27272727272727,7.02272727272727,8.22727272727273,7.88636363636364,8.34090909090909,7.72727272727273,7.90909090909091,7.84090909090909,8.56818181818182,8.22727272727273,8.13636363636364,8.18181818181818,8.88636363636364,9.65909090909091,9.34090909090909,9.50000000000000,9.11363636363636,9.20454545454546,9.27272727272727,9.04545454545455,9.29545454545455,10.4090909090909,10.0454545454545,10.2954545454545,11.0681818181818,10.3636363636364,11.0681818181818,10.5000000000000,10.2500000000000,10.1363636363636,9.52272727272727,11.4090909090909,11.4318181818182,11.5681818181818,11.9772727272727,12.2272727272727,12.5227272727273,12.1590909090909,12.4545454545455,11.3863636363636,12.0681818181818,13.5454545454545,13.1136363636364,11.8181818181818,13.3863636363636,13.2045454545455,13.8636363636364,12.8409090909091,13.0454545454545,13.2045454545455,13.1363636363636,13.3181818181818,13.8409090909091,14.7954545454545,15.3409090909091,14.3636363636364,15.8409090909091,14.5000000000000,14.9545454545455,14.2500000000000,16.2727272727273,14.6136363636364,14.6136363636364,15.6363636363636,17.3181818181818,17.0454545454545,16.4545454545455,16.3636363636364,16.9318181818182,16.3181818181818,17.3863636363636,16.9545454545455,17.2045454545455,15.7272727272727,16.0681818181818,18.0909090909091,18.0681818181818,20.0227272727273,19.8181818181818,17.7954545454545,17.7954545454545,18.9318181818182,18.7954545454545,19.2954545454545,18.6363636363636,18.4090909090909,19.5681818181818,22.4318181818182,21.5454545454545,21.2727272727273,21.5909090909091,20.8636363636364,21.0227272727273,20.7272727272727,22.4090909090909,21.0909090909091,21.1590909090909,20.7727272727273,20.2954545454545,24.5681818181818,23.9090909090909,24.4772727272727,24.2045454545455,25.0454545454545,23,23.6363636363636,23.6818181818182,22.5909090909091,24.6590909090909,24.9318181818182,24.6363636363636,22.5681818181818,27.3863636363636,26.5454545454545,25.9772727272727,23.9545454545455,26.7272727272727,27.8181818181818,26.9318181818182,29.3409090909091,27.2045454545455,25.8181818181818,25.9090909090909,26.5227272727273,27.6136363636364,27.4545454545455,27.4090909090909,30.8636363636364,27.8863636363636,32.0681818181818,28.8863636363636,
//                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                  6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,5.92307692307692,6.46153846153846,6.38461538461539,6.61538461538462,5.84615384615385,5.46153846153846,5.92307692307692,7.30769230769231,7.23076923076923,7.23076923076923,6.53846153846154,7.92307692307692,6.53846153846154,6.23076923076923,6.07692307692308,6.69230769230769,7.84615384615385,7.30769230769231,7.76923076923077,6.38461538461539,6.84615384615385,7.38461538461539,8,7.76923076923077,8.53846153846154,8.84615384615385,7.53846153846154,7.84615384615385,8.07692307692308,9.15384615384615,8.07692307692308,9.23076923076923,9.38461538461539,8.23076923076923,9,8,8.15384615384615,10.3846153846154,12.3846153846154,8.69230769230769,9.76923076923077,9.69230769230769,9.07692307692308,9.07692307692308,10,10.6153846153846,10.0769230769231,10.7692307692308,11.6153846153846,10.6923076923077,10.0769230769231,10.2307692307692,10.8461538461538,11.3846153846154,11.3076923076923,12.2307692307692,11.3846153846154,12.5384615384615,12.9230769230769,13.1538461538462,12.2307692307692,11.7692307692308,11,12.1538461538462,13.3846153846154,12.2307692307692,13.7692307692308,14.6153846153846,13.1538461538462,13.5384615384615,14.5384615384615,12.4615384615385,12.5384615384615,12.3846153846154,12.2307692307692,14.6923076923077,16.3846153846154,15.2307692307692,15.9230769230769,15.0769230769231,14.3076923076923,17.6153846153846,14.8461538461538,15.6153846153846,16.3846153846154,15.0769230769231,14.4615384615385,19.0769230769231,16.9230769230769,20.2307692307692,17,17.5384615384615,15.6923076923077,20.3846153846154,16.9230769230769,17.1538461538462,20.6923076923077,20.0769230769231,18.3076923076923,19.7692307692308,19.0769230769231,18.3076923076923,20.4615384615385,22,20.3846153846154,20.7692307692308,17.4615384615385,23.8461538461538,21.9230769230769,22.3846153846154,19.9230769230769,19.0769230769231,19.1538461538462,24.5384615384615,25.4615384615385,21.1538461538462,20.4615384615385,22.3846153846154,25.0769230769231,24.8461538461538,21,22.6153846153846,21.6923076923077,23,26.9230769230769,25.7692307692308,27.6153846153846,23.2307692307692,23.7692307692308,24.0769230769231,23.6153846153846,32.8461538461539,28.6923076923077,27,29.1538461538462,25.5384615384615,26.9230769230769,30.3846153846154,28.1538461538462,26.1538461538462,28.7692307692308,30.0769230769231,28.2307692307692,31.7692307692308,28.7692307692308,34.0769230769231,30.2307692307692,33.1538461538462,31.6923076923077,30.6923076923077,33.2307692307692,34.6923076923077,33.8461538461539,30.7692307692308,30.9230769230769,32.8461538461539,32.7692307692308,30.1538461538462,36.0769230769231,31.0769230769231,33.6923076923077,35.8461538461539,38.6923076923077,37.4615384615385,32.6153846153846,33.3076923076923,37.2307692307692,35.5384615384615,33.3846153846154,37.2307692307692,45.9230769230769,35,39.3846153846154,40.1538461538462,39.3076923076923,
                  6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,6.25174825174825,5.92307692307692,6.46153846153846,6.38461538461539,6.61538461538462,5.84615384615385,5.46153846153846,5.92307692307692,7.30769230769231,7.23076923076923,7.23076923076923,6.53846153846154,7.92307692307692,6.53846153846154,6.23076923076923,6.07692307692308,6.69230769230769,7.84615384615385,7.30769230769231,7.76923076923077,6.38461538461539,6.84615384615385,7.38461538461539,8,7.76923076923077,8.53846153846154,8.84615384615385,7.53846153846154,7.84615384615385,8.07692307692308,9.15384615384615,8.07692307692308,9.23076923076923,9.38461538461539,8.23076923076923,9,8,8.15384615384615,10.3846153846154,12.3846153846154,8.69230769230769,9.76923076923077,9.69230769230769,9.07692307692308,9.07692307692308,10,10.6153846153846,10.0769230769231,10.7692307692308,11.6153846153846,10.6923076923077,10.0769230769231,10.2307692307692,10.8461538461538,11.3846153846154,11.3076923076923,12.2307692307692,11.3846153846154,12.5384615384615,12.9230769230769,13.1538461538462,12.2307692307692,11.7692307692308,11,12.1538461538462,13.3846153846154,12.2307692307692,13.7692307692308,14.6153846153846,13.1538461538462,13.5384615384615,14.5384615384615,12.4615384615385,12.5384615384615,12.3846153846154,12.2307692307692,14.6923076923077,16.3846153846154,15.2307692307692,15.9230769230769,15.0769230769231,14.3076923076923,17.6153846153846,14.8461538461538,15.6153846153846,16.3846153846154,15.0769230769231,14.4615384615385,19.0769230769231,16.9230769230769,20.2307692307692,17,17.5384615384615,15.6923076923077,20.3846153846154,16.9230769230769,17.1538461538462,20.6923076923077,20.0769230769231,18.3076923076923,19.7692307692308,19.0769230769231,18.3076923076923,20.4615384615385,22,20.3846153846154,20.7692307692308,17.4615384615385,23.8461538461538,21.9230769230769,22.3846153846154,19.9230769230769,19.0769230769231,19.1538461538462,24.5384615384615,25.4615384615385,21.1538461538462,20.4615384615385,22.3846153846154,25.0769230769231,24.8461538461538,21,22.6153846153846,21.6923076923077,23,26.9230769230769,25.7692307692308,27.6153846153846,23.2307692307692,23.7692307692308,24.0769230769231,23.6153846153846,32.8461538461539,28.6923076923077,27,29.1538461538462,25.5384615384615,26.9230769230769,30.3846153846154,28.1538461538462,26.1538461538462,28.7692307692308,30.0769230769231,28.2307692307692,31.7692307692308,28.7692307692308,34.0769230769231,30.2307692307692,33.1538461538462,31.6923076923077,30.6923076923077,33.2307692307692,34.6923076923077,33.8461538461539,30.7692307692308,30.9230769230769,32.8461538461539,32.7692307692308,30.1538461538462,36.0769230769231,31.0769230769231,33.6923076923077,35.8461538461539,38.6923076923077,37.4615384615385,32.6153846153846,33.3076923076923,37.2307692307692,35.5384615384615,33.3846153846154,37.2307692307692,45.9230769230769,35,39.3846153846154,40.1538461538462,39.3076923076923,
                  7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.32411067193676,7.30434782608696,7.13043478260870,6.95652173913044,6.60869565217391,6.95652173913044,6.82608695652174,6.91304347826087,7.43478260869565,6.86956521739131,7.73913043478261,7.08695652173913,8.08695652173913,7.39130434782609,8,8.43478260869565,8.21739130434783,7.21739130434783,7.78260869565217,8.04347826086957,7.30434782608696,7.21739130434783,8.73913043478261,8.52173913043478,8.78260869565217,9.52173913043478,8.91304347826087,8.04347826086957,9.08695652173913,7.65217391304348,9.08695652173913,10.3913043478261,9.08695652173913,9.17391304347826,9.78260869565217,8.95652173913044,9.39130434782609,9.47826086956522,8.65217391304348,10.6086956521739,9.95652173913044,10.1304347826087,10.9565217391304,10.4347826086957,10.8695652173913,9.17391304347826,10.0434782608696,11.3478260869565,11.9565217391304,11.9565217391304,11.1304347826087,12.5217391304348,11.7826086956522,12,12.9130434782609,11.6956521739130,12.9130434782609,13.8260869565217,13.2173913043478,13.2608695652174,14.5652173913043,14,12.6086956521739,15.3478260869565,13.3043478260870,14,15.0434782608696,15.5217391304348,14.7391304347826,15.2173913043478,14.9130434782609,14.6956521739130,14.3043478260870,16.2608695652174,15.8260869565217,14.6521739130435,18.7826086956522,16.2173913043478,16.3913043478261,17.9130434782609,16.8695652173913,17.0869565217391,16.8695652173913,16.6086956521739,16.1739130434783,15.8260869565217,17.0434782608696,17.5652173913043,19.6956521739130,19,18.6521739130435,18.2173913043478,17.9130434782609,18.9130434782609,17.2608695652174,20.6956521739130,18,21.6521739130435,20.8695652173913,21.7391304347826,22.7391304347826,21.8260869565217,21.8260869565217,22.2608695652174,22.9130434782609,21.6956521739130,21.1304347826087,21.7826086956522,23.3043478260870,24.7826086956522,25.6521739130435,26,23,23.2173913043478,23.8260869565217,20.0869565217391,22.8695652173913,22.6956521739130,23,23.7391304347826,24.6086956521739,25.9130434782609,29.4347826086957,26.3913043478261,26.5652173913043,28.2608695652174,26.5652173913043,25.7826086956522,29.6521739130435,27.1739130434783,25.6956521739130,27.8695652173913,27.6086956521739,32.4782608695652,30.6521739130435,31.4347826086957,29.7391304347826,28.2608695652174,30.8260869565217,30.6956521739130,29.4782608695652,30.1739130434783,28,28.4347826086957,35.6956521739131,31.5217391304348,33.2608695652174,31.5652173913043,34.2173913043478,36.4347826086957,30.8695652173913,34.8260869565217,33.4347826086957,35.4782608695652,32.6956521739131,33.7391304347826,33.5652173913044,36.4347826086957,35.5652173913044,37.4347826086957,33.9565217391304,40.2608695652174,38.5217391304348,35.9130434782609,41.2608695652174,38.2173913043478,38,36.3478260869565,37.3913043478261,34.5217391304348,36.6086956521739,45.1739130434783,49.6086956521739,44.2173913043478,40.8695652173913,42.6521739130435,
                  17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,17.4976076555024,16.1578947368421,14.9473684210526,15.8421052631579,15.1578947368421,12.9473684210526,13.9473684210526,14.3684210526316,14.7894736842105,13.8421052631579,13.5789473684211,14.1052631578947,14,13.1578947368421,13.5789473684211,15.2105263157895,12.3684210526316,15.2105263157895,13.5789473684211,14.2631578947368,14.6842105263158,12.8421052631579,13.8421052631579,14.8421052631579,16.2631578947368,14.6842105263158,15.1578947368421,14.7368421052632,14.9473684210526,14.7368421052632,15.9473684210526,17.4210526315790,17.5263157894737,17.0526315789474,15.4736842105263,17.1052631578947,17.9473684210526,16.0526315789474,17.6842105263158,18.6842105263158,18.2105263157895,19.1578947368421,19.0526315789474,18.0526315789474,18.4210526315790,16.3684210526316,19.6842105263158,20.6842105263158,19.6842105263158,18.5789473684211,19.2631578947368,18.6842105263158,18.8947368421053,18.9473684210526,19.6842105263158,18.3157894736842,18.2631578947368,18.7368421052632,18.4736842105263,17.4210526315790,19.8421052631579,19.2631578947368,20.0526315789474,21.1578947368421,18.8947368421053,19.7894736842105,23.7368421052632,22.6842105263158,22,21.1052631578947,19.9473684210526,21.4210526315790,22,22.3684210526316,19.5789473684211,19.4210526315790,22.2105263157895,23.5263157894737,24.9473684210526,21.4210526315790,22.3684210526316,22.0526315789474,21.5789473684211,21.9473684210526,20.4210526315790,21.7368421052632,22.4736842105263,23.9473684210526,24.9473684210526,24.8947368421053,25.2105263157895,28.3684210526316,24.4736842105263,30.1052631578947,23.5263157894737,25.7894736842105,25.8947368421053,27.1578947368421,31.6842105263158,27.2105263157895,29.5263157894737,27.3157894736842,28.5263157894737,27.7894736842105,28.5789473684211,24.4210526315790,28.9473684210526,27.2631578947368,29.5263157894737,29.3684210526316,31.3684210526316,28.3157894736842,30.7368421052632,30.1052631578947,31.7894736842105,31.8947368421053,31.2105263157895,30.6842105263158,32.6315789473684,32.8947368421053,33.6315789473684,35.4210526315789,35.3684210526316,37.3157894736842,33.0526315789474,31.9473684210526,35.6315789473684,36.5789473684211,34.5789473684211,37.1052631578947,34.2631578947368,35.8421052631579,37.6315789473684,37.7894736842105,35.6315789473684,40.1052631578947,39.5789473684211,40.0526315789474,34.0526315789474,36.1052631578947,36.3157894736842,39,34.1052631578947,41.2631578947368,39.3157894736842,45.3684210526316,43.0526315789474,45.7894736842105,42.5263157894737,40.8947368421053,44.7368421052632,43.6842105263158,43.4736842105263,43,42.8947368421053,44.4736842105263,41.7368421052632,50.2105263157895,47.7894736842105,50.6315789473684,50.6842105263158,46.7368421052632,48.7894736842105,49.2105263157895,48.2105263157895,47.8421052631579,47.7894736842105,49.9473684210526,46.5789473684211,57.5789473684211,51.8421052631579,54.9473684210526,58,58.1052631578947,55.5789473684211,50.2631578947368,
                  9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,9.69090909090909,10.1500000000000,9.55000000000000,8.60000000000000,10.8000000000000,8.70000000000000,9,10.4500000000000,9.55000000000000,8.50000000000000,9.45000000000000,9.95000000000000,8.50000000000000,9.90000000000000,10.2000000000000,10.2000000000000,10.1500000000000,9.45000000000000,11.4000000000000,11.9500000000000,9.65000000000000,10.3500000000000,11.8500000000000,12.6000000000000,10.9000000000000,10.7000000000000,10.4500000000000,10.9500000000000,11.3000000000000,10.8000000000000,11.1000000000000,13.0500000000000,14.6000000000000,12.7500000000000,12.4500000000000,12.1500000000000,12.5500000000000,12.9500000000000,13.2000000000000,14.3500000000000,14.3500000000000,14.0500000000000,12.8000000000000,14.1500000000000,13.2500000000000,13.8500000000000,12.2500000000000,15.1000000000000,18.3500000000000,16.2000000000000,16.0500000000000,16.1000000000000,15.9000000000000,15,15.3000000000000,14.6500000000000,18.1000000000000,18.5000000000000,18.4000000000000,17.6000000000000,18,17.8500000000000,17.8500000000000,17.9000000000000,16.3000000000000,17.9000000000000,18.7500000000000,19.7500000000000,19.3500000000000,21.2000000000000,21,17.9500000000000,19.1000000000000,19.2500000000000,18.4000000000000,21.0500000000000,20.0500000000000,21.3500000000000,21.0500000000000,22.1500000000000,22.9000000000000,23.3000000000000,21.9500000000000,21.8500000000000,20.9500000000000,21.1000000000000,24,23.2500000000000,23.3500000000000,24.7500000000000,24.1000000000000,24.2500000000000,24.8000000000000,24.7500000000000,26.3500000000000,25.8500000000000,23.3000000000000,26.3500000000000,28.0500000000000,25.1500000000000,25.4000000000000,24.8500000000000,30.2500000000000,27.4500000000000,25.2500000000000,30.5500000000000,26.7500000000000,28.4500000000000,28.5000000000000,30.2500000000000,31.8500000000000,32.1500000000000,28.6000000000000,28.1500000000000,29.6500000000000,31.1000000000000,29.3500000000000,34.1000000000000,30.2000000000000,35.3500000000000,34,35.9000000000000,35.3000000000000,32.2000000000000,36.9000000000000,32.2500000000000,31.8000000000000,35.1000000000000,28.2000000000000,38.5000000000000,35.5000000000000,35.2500000000000,38.7500000000000,38.5000000000000,40.9500000000000,40.4000000000000,38.8000000000000,38.8500000000000,37.8000000000000,40.6500000000000,35.3500000000000,39.1000000000000,38.3000000000000,36.1000000000000,39.1000000000000,41.3000000000000,46.8000000000000,48.7500000000000,46,45.2000000000000,44.7000000000000,49.0500000000000,47.8000000000000,42.8500000000000,42.7500000000000,41.5500000000000,42.7000000000000,46.6500000000000,52,54.6500000000000,50.4500000000000,47.1000000000000,49.8000000000000,53.2000000000000,50.3500000000000,48.4500000000000,46.0500000000000,44.6500000000000,49.4500000000000,48.5500000000000,51.6500000000000,54.5000000000000,55.9500000000000,56.2500000000000,59.5500000000000,58.2000000000000,
                  16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.8771043771044,16.7222222222222,17.7777777777778,16.2962962962963,16.7592592592593,16.0370370370370,16.6111111111111,15.5185185185185,17.5370370370370,16.6111111111111,17.3518518518519,15.2592592592593,15.2222222222222,14.9629629629630,15.2222222222222,17.4814814814815,17.2777777777778,15.8888888888889,16.4444444444444,15.2037037037037,14.8148148148148,15.9629629629630,17.0185185185185,17.7222222222222,16.7777777777778,16.3518518518519,16.4814814814815,17.0740740740741,16.3888888888889,14.6851851851852,18.5370370370370,18.8148148148148,17.7037037037037,19.2777777777778,17.8888888888889,16.8888888888889,17.7592592592593,17.2222222222222,19.8518518518519,20.8518518518519,18.9074074074074,18.8148148148148,19.7592592592593,18.2407407407407,18.7592592592593,18.1666666666667,18.0370370370370,20.8148148148148,20.7777777777778,22.1481481481482,21.7407407407407,20.9259259259259,21.8148148148148,23.1851851851852,21.8888888888889,22,23.8703703703704,24.2777777777778,24.7222222222222,24.6296296296296,23.7037037037037,23.1111111111111,23.1851851851852,22.3888888888889,23.2777777777778,23.6296296296296,26.5000000000000,24.9444444444444,27.1481481481482,29.3333333333333,24.8148148148148,24.6111111111111,27.2592592592593,25.2222222222222,25.4444444444444,25.9629629629630,29.7962962962963,30.7777777777778,29.7407407407407,29.7037037037037,27.9444444444444,31.3888888888889,28.8148148148148,28.8148148148148,30.7592592592593,30.2777777777778,30.8148148148148,31.8703703703704,33.0925925925926,32.7222222222222,31.7777777777778,33.7592592592593,30.7592592592593,33.3888888888889,31.6296296296296,32.2037037037037,33.3703703703704,34.2592592592593,36.9074074074074,37.1851851851852,36.0370370370370,35.7407407407407,36.7777777777778,34.7962962962963,38.2962962962963,35.1296296296296,34.7777777777778,34.6851851851852,42.3703703703704,38.2777777777778,38.8888888888889,38.8333333333333,39.8333333333333,37.2222222222222,43.4629629629630,37.9629629629630,40.9629629629630,39.3333333333333,40.5370370370370,38.0370370370370,42.6111111111111,46.1481481481481,44.8333333333333,47.4444444444444,46.2222222222222,45.8518518518519,43.4629629629630,44.3888888888889,45.0370370370370,43.8518518518519,43.3518518518519,43.8333333333333,51.3518518518519,54,47.0925925925926,48.6481481481481,48.1851851851852,48.7592592592593,48.1296296296296,47.2037037037037,49.7407407407407,48.7407407407407,49.2407407407407,52.3888888888889,54.0925925925926,57.0185185185185,56.9444444444444,55.0925925925926,54.6111111111111,56.0555555555556,57.5000000000000,55.9814814814815,55.3518518518519,57.5555555555556,55.7407407407407,54.0370370370370,56.5185185185185,60.8148148148148,61.2592592592593,65.8333333333333,61.7962962962963,61.1296296296296,64.5740740740741,65.4074074074074,60.1481481481481,63.6296296296296,61.8333333333333,61.4814814814815,64.5925925925926,61.1851851851852,66.3518518518519,70.5740740740741,69.1111111111111,69.2222222222222,71.4444444444444,71.5925925925926,
                  13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.1590909090909,13.8000000000000,13.3750000000000,13.8000000000000,12.7500000000000,13.6250000000000,13.3250000000000,12.5500000000000,15,14.2750000000000,14.5500000000000,13.7250000000000,15.1750000000000,14.5500000000000,14.8500000000000,16.4000000000000,16.4250000000000,16.4000000000000,16,15.4000000000000,16.3500000000000,16.6000000000000,17.1000000000000,17.5500000000000,16.9000000000000,17.6250000000000,17.4250000000000,16.4250000000000,15.9250000000000,16.6750000000000,18.6750000000000,18.1500000000000,18.8250000000000,19.1750000000000,18.2500000000000,17.8750000000000,18.8250000000000,18.1000000000000,20.1000000000000,19.8250000000000,19.4500000000000,21.4250000000000,19.0750000000000,20.6750000000000,19.5750000000000,20.7750000000000,20.7500000000000,22.1000000000000,23.5500000000000,21.4750000000000,21.6500000000000,23.3000000000000,20.5000000000000,22.3000000000000,22.2750000000000,23.0500000000000,23.8250000000000,25.3250000000000,23.9500000000000,25.6250000000000,25.5750000000000,25.0500000000000,23.4500000000000,25.2250000000000,26.8500000000000,24.6250000000000,28.6500000000000,29.6250000000000,27.4250000000000,26.0750000000000,27.6000000000000,28.0750000000000,26.9250000000000,26.7000000000000,26.9500000000000,27.8750000000000,30.4750000000000,30.1500000000000,28.5750000000000,31.6500000000000,30.5250000000000,31.8250000000000,31.3000000000000,31.2750000000000,30.1250000000000,29.9750000000000,32.0500000000000,33.8500000000000,33.7750000000000,33.8000000000000,34.6500000000000,32.9250000000000,34.3750000000000,34.9000000000000,38.8250000000000,35.6500000000000,36.0250000000000,39.4250000000000,37.7750000000000,37.6750000000000,37.6250000000000,36.7250000000000,35.5750000000000,36.0250000000000,38.5000000000000,38.6250000000000,37.2500000000000,38.1250000000000,40.7000000000000,43.0500000000000,41.3750000000000,43.9500000000000,41.1500000000000,45.2750000000000,44.2500000000000,41.9500000000000,39.9500000000000,42,44.1250000000000,42.6750000000000,48.4000000000000,50.4000000000000,48.2750000000000,48.5750000000000,48.9500000000000,46.7000000000000,46.9750000000000,47.9000000000000,46.4250000000000,47.3500000000000,46.5500000000000,46.4000000000000,51.1000000000000,54.2000000000000,52.3250000000000,57.4500000000000,58.1500000000000,51.2250000000000,52.8500000000000,56.2500000000000,52.5250000000000,52.9750000000000,57.1250000000000,54.9500000000000,58.3000000000000,60,59.4250000000000,59.4500000000000,61.4250000000000,58.7750000000000,62,58.4750000000000,63.1500000000000,62.1750000000000,61.6750000000000,58.4500000000000,64.1250000000000,64.6500000000000,67.0750000000000,67.6250000000000,67.1250000000000,64.8750000000000,71.3250000000000,66.4250000000000,65.7000000000000,65.6000000000000,67.0250000000000,72.8250000000000,67.4750000000000,67.3750000000000,68.0500000000000,71.2000000000000,72.8500000000000,76.3000000000000,70.7500000000000,71.7750000000000
              };
    CvMat mat = cvMat( 16, 250, CV_32FC1, a );
    noiseMat = cv::Mat(&mat,true);
    double startD=10.0/50.0*250.0;
    double factor=3.0;
//    qDebug() << "Rows " << noiseMat.rows << " Cols " << noiseMat.cols;
    for(int i=0; i<noiseMat.rows;i++)
    {
        float max = 0.0;
        for(int k=0;k<250;k++)
        {
            if(noiseMat.at<float>(i,k) > max)
                max = noiseMat.at<float>(i,k);
        }
        if(max > 0.0)
        {
            float nearNoise = 0.0;
            for(int k=0;k<250;k++)
            {
                float tmpNoise = ((startD - k+1) / (startD*factor));
                if(tmpNoise > nearNoise)
                    nearNoise = tmpNoise;
            }
            for(int j=0;j<noiseMat.cols;j++)
                if(noiseMat.at<float>(i,j) < nearNoise)
                    noiseMat.at<float>(i,j) = nearNoise;
        }
    }
}

