#include "sonarechodata.h"

SonarEchoData::SonarEchoData()
{
    this->raw = QByteArray(250,0);
    this->filtered = QByteArray(250,0);
    this->classLabel = 0;
    this->wallCandidate = -1;

}

SonarEchoData::SonarEchoData(SonarReturnData data)
{
    QByteArray arr = data.getEchoData();
    arr.chop(2);
    this->raw = arr;
    arr.clear();
    this->filtered.clear();
    this->classLabel = -1;
    this->wallCandidate = -1;
    this->headPosition = data.getHeadPosition();
    this->range = data.getRange();
    this->gain = data.switchCommand.startGain;
    this->group = -1;
    this->features = cv::Mat(1,9,CV_32F);
    this->timestamp = data.switchCommand.time;

}

SonarEchoData::SonarEchoData(const SonarEchoData& dat)
{
    this->raw = dat.raw;
    this->classLabel = dat.classLabel;
    this->filtered = dat.filtered;
    this->wallCandidate = dat.wallCandidate;
    this->headPosition = dat.headPosition;
    this->range = dat.range;
    this->timestamp = dat.timestamp;
    this->features = dat.features;
    this->gain = dat.gain;
    this->group = dat.group;
}

cv::Mat SonarEchoData::getFeatures()
{
//    for(int i=0; i<features.cols;i++)
//        qDebug() << "FEAT " << features.at<float>(0,i);
    return this->features;
}

int SonarEchoData::getClassLabel()
{
    return this->classLabel;
}

int SonarEchoData::getWallCandidate()
{
    return this->wallCandidate;
}

QByteArray SonarEchoData::getRawData() const
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

int SonarEchoData::getGroupID()
{
    return this->group;
}

void SonarEchoData::setGroupID(int id)
{
    this->group = id;
}

void SonarEchoData::addFeature(int index, float value)
{
//    qDebug() << "feature added " << value;
    features.at<float>(0,index) = value;
}

void SonarEchoData::setClassLabel(int label)
{
   if(label > 0)
        this->classLabel = 1;
   else
       this->classLabel = 0;
}

void SonarEchoData::setFiltered(QByteArray data)
{
    this->filtered = data;
}

void SonarEchoData::setWallCandidate(int bin)
{
    this->wallCandidate = bin;
}

QVector2D SonarEchoData::getEuclidean()
{
    double x,y;
    float range = this->getRange();
    x = cos(this->getHeadPosition()/180*M_PI)*this->getWallCandidate()*range/N;
    y = sin(this->getHeadPosition()/180*M_PI)*this->getWallCandidate()*range/N;
    QVector2D vec = QVector2D(x,y);
    return vec;
}

void SonarEchoData::addOffsetToHeadPos(float degree)
{
    this->headPosition = this->headPosition + degree;
}

