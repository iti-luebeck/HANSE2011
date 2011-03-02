#include "sonarechodata.h"

SonarEchoData::SonarEchoData(SonarReturnData data)
{
    QByteArray arr = data.getEchoData();
    arr.chop(2);
    this->raw = arr;
    arr.clear();
    this->filtered.clear();
    this->classLabel = 0;
    this->bClassified = false;
    this->bFiltered = false;
    this->bWallCandidate = false;
    this->wallCandidate = -1;
    this->headPosition = data.getHeadPosition();
    this->range = data.getRange();
    this->gain = data.switchCommand.startGain;

    this->features.clear();
    this->timestamp = data.switchCommand.time;

}

QByteArray SonarEchoData::getFeatures()
{
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

void SonarEchoData::addFeature(float value)
{
    features.append(value);
}

void SonarEchoData::setClassLabel(bool isWallCand)
{
    this->classLabel = 1;
    if(!isWallCand)
        this->classLabel = -1;
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
    this->bWallCandidate = true;
}


