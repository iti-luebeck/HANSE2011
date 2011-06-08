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
    QList<float> getGradient();
    int getWallCandidate();
    float getRange();
    float getHeadPosition();
    float getGain();
    float getHeadingIncrement();
    QDateTime getTimeStamp();
    float getCompassHeading();

    void setFiltered(QByteArray data);
    void setGradient(QList<float> data);
    void setWallCandidate(int bin);
    void addOffsetToHeadPos(float degree);
    void setHeadingIncrement(float inc);
    void setCompassHeading(float head);

    QVector2D getEuclidean();

    const static int N = 250;


private:
    QByteArray raw;
    QByteArray filtered;
    QList<float> gradient;

    int wallCandidate;
    float headPosition;
    float range;
    QDateTime timestamp;
    float gain;
    float headingIncrement;
    float compassHeading;
};

#endif // SONARECHODATA_H
