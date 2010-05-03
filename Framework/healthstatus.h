#ifndef HEALTHSTATUS_H
#define HEALTHSTATUS_H

#include <QtCore>
#include "Framework_global.h"

class RobotModule;


class FRAMEWORKSHARED_EXPORT HealthStatus
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
