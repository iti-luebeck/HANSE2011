#ifndef SONARECHODATA_H
#define SONARECHODATA_H

#include <QByteArray>
#include <QDateTime>
#include "Module_ScanningSonar/sonarreturndata.h"
#include <opencv/cv.h>

#define POSITIVE_CLASS  1;
#define NEGATIVE_CLASS  0;

class SonarEchoData
{
public:
    SonarEchoData(SonarReturnData data);
    SonarEchoData(const SonarEchoData& dat);
    SonarEchoData();

    QByteArray getRawData() const;
    QByteArray getFiltered();
    QByteArray getGradient();
    int getWallCandidate();
    int getClassLabel();
    float getRange();
    float getHeadPosition();
    float getGain();
    QDateTime getTimeStamp();
    cv::Mat getFeatures();

    void setClassLabel(int label);
    void setFiltered(QByteArray data);
    void setGradient(QByteArray data);
    void setWallCandidate(int bin);
    void addFeature(int index, float value);
    void addOffsetToHeadPos(float degree);

    void setGroupID(int id);
    int getGroupID();

    QVector2D getEuclidean();

    const static int N = 250;


private:
    QByteArray raw;
    int classLabel;
    QByteArray filtered;
    QByteArray gradient;
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
