#include "robotmodule.h"

RobotModule::RobotModule(QString newId)
    : QObject(), id(newId), settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse")
{
    settings.beginGroup(id);
    setDefaultValue("enabled", true);
    logger = Log4Qt::Logger::logger(id);

    // perform a health check once a second
    connect(&healthCheckTimer, SIGNAL(timeout()), this, SLOT(doHealthCheck()));
    healthCheckTimer.start(1000);
}

void RobotModule::setEnabled(bool value)
{
    settings.setValue("enabled", value);

    emit enabled(settings.value("enabled").toBool());
}

bool RobotModule::isEnabled()
{
    return settings.value("enabled").toBool();
}


QString RobotModule::getTabName()
{
    return id;
}

QSettings& RobotModule::getSettings()
{
    return settings;
}

void RobotModule::setDefaultValue(const QString &key, const QVariant &value)
{
    if (!settings.contains(key))
        settings.setValue(key, value);
}

void RobotModule::setHealthToOk()
{
    logger->info("Health status changed: Back to healthy!");
    healthStatus.healthOk = true;
    emit healthStatusChanged(this);
}

void RobotModule::setHealthToSick(QString errorMsg)
{
    logger->error("Health status changed: Sick!");
    logger->error("Health status: Last error message: "+errorMsg);
    healthStatus.healthOk = false;
    healthStatus.errorCount++;
    healthStatus.lastError = errorMsg;
    emit healthStatusChanged(this);
}

HealthStatus RobotModule::getHealthStatus()
{
    return healthStatus;
}

void RobotModule::doHealthCheck()
{
    printf("YYYYYYY\n");
}
