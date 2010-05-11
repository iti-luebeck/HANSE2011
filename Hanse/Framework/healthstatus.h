#ifndef HEALTHSTATUS_H
#define HEALTHSTATUS_H

#include <QtCore>

class RobotModule;

class HealthStatus
{
    friend class RobotModule;
public:
    HealthStatus();

    bool isHealthOk();
    QString getLastError();
    int getErrorCount();

private:
    bool healthOk;
    QString lastError;
    int errorCount;
};

#endif // HEALTHSTATUS_H
