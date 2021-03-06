#include "sonarechodata.h"

SonarEchoData::SonarEchoData()
{
    this->raw = QByteArray(250,0);
    this->filtered = QByteArray(250,0);
    this->gradient = QList<float>();
    for (int i = 0; i < 250; i++) this->gradient.append(0.0f);
    this->wallCandidate = -1;
    this->compassHeading = 0;
}

SonarEchoData::SonarEchoData(SonarReturnData data)
{
    QByteArray arr = data.getEchoData();
    arr.chop(2);
    this->raw = arr;
    arr.clear();
    this->filtered.clear();
    this->gradient.clear();
    this->wallCandidate = -1;
    this->headPosition = data.getHeadPosition();
    this->range = data.getRange();
    this->gain = data.switchCommand.startGain;
//    this->features = cv::Mat(1,9,CV_32F);
    this->timestamp = data.switchCommand.time;
    this->compassHeading = 0;
}

SonarEchoData::SonarEchoData(const SonarEchoData& dat)
{
    this->raw = dat.raw;
    this->filtered = dat.filtered;
    this->wallCandidate = dat.wallCandidate;
    this->headPosition = dat.headPosition;
    this->range = dat.range;
    this->timestamp = dat.timestamp;
    this->gain = dat.gain;

    this->gradient.clear();
    for (int i = 0; i < dat.gradient.size(); i++) {
        this->gradient.push_back(dat.gradient[i]);
    }
    this->compassHeading = dat.compassHeading;
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

QList<float> SonarEchoData::getGradient()
{
    return this->gradient;
}

QDateTime SonarEchoData::getTimeStamp()
{
    return this->timestamp;
}

float SonarEchoData::getHeadPosition()
{
    return this->headPosition + 180;
}

float SonarEchoData::getRange()
{
    return this->range;
}

float SonarEchoData::getGain()
{
    return this->gain;
}

void SonarEchoData::setFiltered(QByteArray data)
{
    this->filtered = data;
}

void SonarEchoData::setGradient(QList<float> data)
{
    this->gradient.clear();
    for (int i = 0; i < data.size(); i++) {
        this->gradient.push_back(data[i]);
    }
}

void SonarEchoData::setWallCandidate(int bin)
{
    this->wallCandidate = bin;
}

QVector2D SonarEchoData::getEuclidean(float headingOffset)
{
    double x,y;
    float range = this->getRange();
    x = cos((this->getHeadPosition() + headingOffset) / 180 * M_PI) * this->getWallCandidate() * range / N;
    y = sin((this->getHeadPosition() + headingOffset) / 180 * M_PI) * this->getWallCandidate() * range / N;
    QVector2D vec = QVector2D(x,y);
    return vec;
}

void SonarEchoData::addOffsetToHeadPos(float degree)
{
    this->headPosition = this->headPosition + degree;
}

float SonarEchoData::getCompassHeading()
{
    return compassHeading;
}

void SonarEchoData::setCompassHeading(float head)
{
    this->compassHeading = head;
}

