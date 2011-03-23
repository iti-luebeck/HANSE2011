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

    void setClassLabel(int label);
    void setFiltered(QByteArray data);
    void setWallCandidate(int bin);
    void addFeature(int index, float value);

    void setGroupID(int id);
    int getGroupID();



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
    int group;
};

#endif // SONARECHODATA_H
