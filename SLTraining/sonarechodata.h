#ifndef SONARECHODATA_H
#define SONARECHODATA_H

#include <QByteArray>
#include <QDateTime>
#include "sonarreturndata.h"
#include <opencv/cv.h>

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
    cv::Mat getFeatures();

    void setClassLabel(bool isWallCand);
    void setFiltered(QByteArray data);
    void setWallCandidate(int bin);
    void addFeature(int index, float value);



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
//    QByteArray features;
    cv::Mat features;
    float gain;
//    float features;
};

#endif // SONARECHODATA_H
