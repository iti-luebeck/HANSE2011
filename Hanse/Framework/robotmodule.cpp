#include "robotmodule.h"
#include "datarecorder.h"

RobotModule::RobotModule(QString newId)
    : QObject(), id(newId), settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse")
{
    settings.beginGroup(id);

    setDefaultValue("enabled", true);
    setDefaultValue("enableLogging", true);

    logger = Log4Qt::Logger::logger(id);

    // perform a health check once a second
    connect(&healthCheckTimer, SIGNAL(timeout()), this, SLOT(doHealthCheck()));
    healthCheckTimer.start(1000);

    recorder = new DataRecorder(*this);
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

QString RobotModule::getId()
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
    if (!healthStatus.isHealthOk()) {
        logger->info("Health OK");
        healthStatus.healthOk = true;
        emit healthStatusChanged(this);
    }
}

void RobotModule::setHealthToSick(QString errorMsg)
{
    healthStatus.errorCount++;
    if (healthStatus.isHealthOk() || healthStatus.getLastError() != errorMsg) {
        logger->error("Health ERROR: "+errorMsg);
        healthStatus.healthOk = false;
        healthStatus.lastError = errorMsg;
        // each time this signal is emitted, a line will be written to the logfile.
        // thus don't emit this signal when the errormsg stays the same.
        emit healthStatusChanged(this);
    }
}

HealthStatus RobotModule::getHealthStatus()
{
    return healthStatus;
}

void RobotModule::doHealthCheck()
{
    // Needs to be reimplemented in subclasses
}

const QMap<QString,QVariant> RobotModule::getData() {
    return data;
}

void RobotModule::terminate()
{
    recorder->close();
}

void RobotModule::reset()
{
    healthStatus.errorCount=0;
    healthStatus.lastError="";
    healthStatus.healthOk=true;
}
