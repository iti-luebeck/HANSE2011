#include "robotmodule.h"
#include "datarecorder.h"

RobotModule::RobotModule(QString newId)
    : settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse"),
      moduleMutex(QMutex::Recursive),
      id(newId),
      healthCheckTimer(this)
{
    settings.beginGroup(id);

    setDefaultValue("enabled", true);
    setDefaultValue("enableLogging", true);

    logger = Log4Qt::Logger::logger(id);

    // perform a health check once a second
    connect(&healthCheckTimer, SIGNAL(timeout()), this, SLOT(doHealthCheck()),Qt::DirectConnection);
    healthCheckTimer.start(1000);

    recorder = new DataRecorder(*this);
}

void RobotModule::setEnabled(bool value)
{
    settings.setValue("enabled", value);

    if (value)
        reset();

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

//QSettings& RobotModule::getSettings()
//{
//    return settings;
//}

const  QMap<QString,QVariant> RobotModule::getSettingsCopy()
{
    QMutexLocker l(&dataLockerMutex);
    QMap<QString,QVariant> map;

    QStringList keys = settings.allKeys();

    foreach (QString key, keys) {
       map.insert(key,settings.value(key));
    }
    return map;
//    return settings;
}

QStringList RobotModule::getSettingKeys()
{
    QMutexLocker l(&dataLockerMutex);
    QStringList list = settings.allKeys();
    return list;
}


const QVariant RobotModule::getSettingsValue(const QString key, const QVariant defValue)
{
    QMutexLocker l(&dataLockerMutex);
    QVariant qv = settings.value(key,defValue);
    return qv;
}

const QVariant RobotModule::getSettingsValue(const QString key)
{
    QMutexLocker l(&dataLockerMutex);
    QVariant qv = settings.value(key);
    return qv;
}

void RobotModule::setSettingsValue(QString key, QVariant value)
{
    QMutexLocker l(&dataLockerMutex);
    settings.setValue(key,value);
}


void RobotModule::setDefaultValue(const QString &key, const QVariant &value)
{
    QMutexLocker l(&dataLockerMutex);
    if (!settings.contains(key))
        settings.setValue(key, value);
}

void RobotModule::setHealthToOk()
{
    healthStatusMutex.lock();
    if (!healthStatus.isHealthOk()) {
        logger->info("Health OK");
        healthStatus.healthOk = true;
        healthStatusMutex.unlock();
        emit healthStatusChanged(this);
    } else {
        healthStatusMutex.unlock();

    }
}

void RobotModule::setHealthToSick(QString errorMsg)
{
    healthStatusMutex.lock();
    healthStatus.errorCount++;
    if (healthStatus.isHealthOk() || healthStatus.getLastError() != errorMsg) {
        logger->error("Health ERROR: "+errorMsg);
        healthStatus.healthOk = false;
        healthStatus.lastError = errorMsg;
        healthStatusMutex.unlock();
        // each time this signal is emitted, a line will be written to the logfile.
        // thus don't emit this signal when the errormsg stays the same.
        emit healthStatusChanged(this);
    } else {
        healthStatusMutex.unlock();
    }
}

HealthStatus RobotModule::getHealthStatus()
{
    QMutexLocker l(&healthStatusMutex);
    return healthStatus;
}

void RobotModule::doHealthCheck()
{
    // Needs to be reimplemented in subclasses
}

const QMap<QString,QVariant> RobotModule::getDataCopy() {
    QMutexLocker l(&dataLockerMutex);
    QMap<QString,QVariant> map;
    QMap<QString, QVariant>::const_iterator i = data.constBegin();
    while (i != data.constEnd()) {
        map.insert(i.key(),i.value());
        ++i;
    }
    return map;
}

const QVariant RobotModule::getDataValue(QString key)
{
    QMutexLocker l(&dataLockerMutex);
    QVariant qv = data[key];
    return qv;
}

void RobotModule::addData(QString key, QVariant value)
{
    QMutexLocker l(&dataLockerMutex);
    data[key] = value;
}

void RobotModule::terminate()
{
    recorder->close();
}

void RobotModule::reset()
{
    data.clear();
    QMutexLocker l(&healthStatusMutex);
    healthStatus.errorCount=0;
    healthStatus.lastError="";
    healthStatus.healthOk=true;
}

void RobotModule::msleep(int millies)
{
    MyQThread::msleep(millies);
}

void RobotModule::MyQThread::msleep(int millies)
{
    QThread::msleep(millies);
}
