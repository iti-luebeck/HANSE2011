#include "sonarechodata.h"

SonarEchoData::SonarEchoData(SonarReturnData data)
{
    QByteArray arr = data.getEchoData();
    arr.chop(2);
    this->raw = arr;
    arr.clear();
    this->filtered.clear();
    this->classLabel = -1;
    this->bClassified = false;
    this->bFiltered = false;
    this->bWallCandidate = false;
    this->wallCandidate = -1;
    this->headPosition = data.getHeadPosition();
    this->range = data.getRange();
    this->gain = data.switchCommand.startGain;

    this->features = cv::Mat(1,9,CV_32F);
    this->timestamp = data.switchCommand.time;

}

cv::Mat SonarEchoData::getFeatures()
{
//    for(int i=0; i<features.cols;i++)
//        qDebug() << "FEAT " << features.at<float>(0,i);
    return this->features;
}

int SonarEchoData::getClassLabel()
{
    if(this->bClassified)
        return this->classLabel;
    return NULL;
}

int SonarEchoData::getWallCandidate()
{
    return this->wallCandidate;
}

bool SonarEchoData::isClassified()
{
    return this->bClassified;
}

bool SonarEchoData::isFiltered()
{
    return this->bFiltered;
}

bool SonarEchoData::hasWallCandidate()
{
    return this->bWallCandidate;
}

QByteArray SonarEchoData::getRawData()
{
    return this->raw;
}

QByteArray SonarEchoData::getFiltered()
{
    return this->filtered;
}

QDateTime SonarEchoData::getTimeStamp()
{
    return this->timestamp;
}

float SonarEchoData::getHeadPosition()
{
    return this->headPosition;
}

float SonarEchoData::getRange()
{
    return this->range;
}

float SonarEchoData::getGain()
{
    return this->gain;
}

void SonarEchoData::addFeature(int index, float value)
{
//    qDebug() << "feature added " << value;
    features.at<float>(0,index) = value;
}

void SonarEchoData::setClassLabel(bool isWallCand)
{
    this->bClassified = false;
    this->classLabel = 1;
    if(!isWallCand)
        this->classLabel = 0;
    if(this->wallCandidate != -1)
        this->bClassified = true;
}

void SonarEchoData::setFiltered(QByteArray data)
{
    this->filtered = data;
    this->bFiltered = true;
}

void SonarEchoData::setWallCandidate(int bin)
{
    this->wallCandidate = bin;
    this->bWallCandidate = false;
    if(bin != -1)
        this->bWallCandidate = true;
}


