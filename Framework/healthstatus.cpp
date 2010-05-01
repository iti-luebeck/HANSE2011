#include "healthstatus.h"

HealthStatus::HealthStatus()
{
    errorCount = 0;
    healthOk = true;
    lastError = "";
}

int HealthStatus::getErrorCount()
{
    return errorCount;
}

bool HealthStatus::isHealthOk()
{
    return healthOk;
}

QString HealthStatus::getLastError()
{
    return lastError;
}
