#include "sonardatasourcerivas.h"
#include "QtCore"
#include "module_scanningsonar.h"
#include "sonarswitchcommand.h"
#include <Framework/Angles.h>
#include <Module_XsensMTi/module_xsensmti.h>

SonarDataSourceRivas::SonarDataSourceRivas(Module_ScanningSonar &parent, Module_XsensMTi *mti, QString sonarPath, QString mtiPath)
    : SonarDataSource(parent)
{
    logger = Log4Qt::Logger::logger("SonarFileReaderRivas");

    this->sonarStream = NULL;
    this->sonarFile = new QFile(sonarPath);
    if (!sonarFile->open(QIODevice::ReadOnly)) {
        logger->error("Could not open file " + sonarFile->fileName() + " for reading.");
        delete sonarFile;
        sonarFile = NULL;
        return;
    }

    this->sonarStream = new QTextStream(sonarFile);
//    this->sonarStream->setLocale(QLocale(QLocale::German, QLocale::Germany));

    this->mtiStream = NULL;
    this->mtiFile = new QFile(mtiPath);
    if (!mtiFile->open(QIODevice::ReadOnly)) {
        logger->error("Could not open file " + mtiFile->fileName() + " for reading.");
        delete mtiFile;
        mtiFile = NULL;
        return;
    }

    this->mtiStream = new QTextStream(mtiFile);
//    this->mtiStream->setLocale(QLocale(QLocale::German, QLocale::Germany));

    currentTime = 0;
    lastTime = 0;
    currentHeading = 0;
    lastHeading = 0;
    this->mti = mti;
}

SonarDataSourceRivas::~SonarDataSourceRivas()
{
    this->stop();
}

const SonarReturnData SonarDataSourceRivas::getNextPacket()
{
    SonarReturnDataRivas p = readPacket();
    parent.msleep(parent.getSettingsValue("fileReaderDelay").toInt());
    return p;
}

const SonarReturnDataRivas SonarDataSourceRivas::getNextPacketRivas()
{
    SonarReturnDataRivas p = readPacket();
    parent.msleep(parent.getSettingsValue("fileReaderDelay").toInt());
    return p;
}

SonarReturnDataRivas SonarDataSourceRivas::readPacket()
{
    if (!sonarStream) {
        logger->error("Stream not open!");
        SonarReturnDataRivas inv;
        return inv;
    }

    if (sonarStream->atEnd()) {
        logger->error("Reached end of file!");
        SonarReturnDataRivas inv;
        return inv;
    }

    double time = 0;
    float sonarTime = 0;
    float headPos = 0;

    *sonarStream >> time >> sonarTime >> headPos;
    QByteArray data;
    for (int i = 0; i < 500; i++) {
        int value = 0;
        *sonarStream >> value;
        data.append((unsigned char)value);
    }

    SonarSwitchCommand cmd;
    cmd.range = 50;
    cmd.startGain = 1;
    cmd.stepSize = 2;
    cmd.dataPoints = 120;

    if (!mtiStream || mtiStream->atEnd()) {
        SonarReturnDataRivas d(cmd, data, headPos * 180 / M_PI);
        return d;
    }

    // Adjust with MTi heading position.
    float roll, pitch, yaw, wx, wy, wz, ax, ay, az;
    while (time > currentTime) {
        lastHeading = currentHeading;
        lastTime = currentTime;
        *mtiStream >> currentTime >> roll >> pitch >> yaw >> wx >> wy >> wz >> ax >> ay >> az;
        currentHeading = yaw;
    }

//    qDebug("%f -- %f, %f", time, lastTime, headPos);

    SonarReturnDataRivas d(cmd, data, headPos * 180 / M_PI);

    if (mti != NULL) {
        mti->addData("yaw", lastHeading);
        mti->addData("own time", lastTime);
        mti->addData("sonar time", time);
    }
//    SonarReturnDataRivas d(cmd, data, Angles::deg2deg(headPos * 180 / M_PI + lastHeading));
    return d;
}

bool SonarDataSourceRivas::isOpen()
{
    return sonarFile && sonarFile->isOpen() && sonarStream && !sonarStream->atEnd();
}

void SonarDataSourceRivas::stop()
{
    if (sonarStream) {
        delete(sonarStream);
        sonarStream = NULL;
    }
    if (sonarFile) sonarFile->close();
    if (mtiStream) {
        delete(mtiStream);
        mtiStream = NULL;
    }
    if (mtiFile) mtiFile->close();
}

