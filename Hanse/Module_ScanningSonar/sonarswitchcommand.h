#ifndef SONARSWITCHCOMMAND_H
#define SONARSWITCHCOMMAND_H

#include <QtCore>
#include <log4qt/logger.h>

class SonarSwitchCommand : public QObject
{
Q_OBJECT
public:
    // when reading from file
    SonarSwitchCommand(QByteArray fileData);

    // when building it by hand (serial)
    SonarSwitchCommand();

    SonarSwitchCommand(const SonarSwitchCommand&);

    QByteArray toSerialCmd();
    SonarSwitchCommand& operator =(SonarSwitchCommand);

    QDateTime time;
    unsigned char range;
    bool reversedDirection;
    unsigned char startGain;
    unsigned char trainAngle;
    unsigned char sectorWidth;
    unsigned char stepSize;
    unsigned char pulseLength;
    unsigned char dataPoints;
    unsigned char switchDelay;
    unsigned char frequency;
    unsigned short totalBytes;
    unsigned short nToRead;


private:
    void extractHeader(const QByteArray& a);
    void clone(const SonarSwitchCommand& other);
    Log4Qt::Logger *logger;

};

#endif // SONARSWITCHCOMMAND_H
