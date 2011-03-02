#ifndef SONARECHODATA_H
#define SONARECHODATA_H

#include <QByteArray>
#include <QDateTime>
#include "sonarreturndata.h"

class SonarEchoData
{
public:
    SonarEchoData(SonarReturnData data);

    QByteArray getRawData();
    QByteArray getFiltered();
    bool isClassified();
    bool hasWallCandidate();
    bool isFiltered();
    int getWallCandidate();
    int getClassLabel();
    float getRange();
    float getHeadPosition();
    float getGain();
    QDateTime getTimeStamp();
    QByteArray getFeatures();

    void setClassLabel(bool isWallCand);
    void setFiltered(QByteArray data);
    void setWallCandidate(int bin);
    void addFeature(float value);


private:
    QByteArray raw;
    bool bClassified;
    bool bWallCandidate;
    bool bFiltered;
    int classLabel;
    QByteArray filtered;
    int wallCandidate;
    float headPosition;
    float range;
    QDateTime timestamp;
    QByteArray features;
    float gain;
//    float features;
};

#endif // SONARECHODATA_H
