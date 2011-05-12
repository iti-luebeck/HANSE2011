#include "robotmodule.h"
#include "datarecorder.h"

RobotModule::RobotModule(QString newId)
    : dataLockerMutex(QMutex::Recursive),
      id(newId),
      healthCheckTimer(this),
      initialized(false)
{
    moveToThread(this);
    setDefaultValue("enabled", true);

    logger = Log4Qt::Logger::logger(id);
    recorder = new DataRecorder(*this);
}

void RobotModule::run()
{
    this->init();
    dataLockerMutex.lock();
    this->initialized = true;
    dataLockerMutex.unlock();
    this->exec();
    this->terminate();

}

void RobotModule::setEnabled(bool value)
{
    setSettingsValue("enabled",value);

    if (value) {
        // this method may be called from other threads, therefore we do the reset
        // asynchronously
        QTimer::singleShot(0, this, SLOT(reset()));
    }
    emit enabled(getSettingsValue("enabled").toBool());

}

bool RobotModule::isEnabled()
{
    return getSettingsValue("enabled").toBool();
}

bool RobotModule::isInitialized()
{
    QMutexLocker l(&dataLockerMutex);
    return this->initialized;
}

QString RobotModule::getTabName()
{
    return id;
}

QString RobotModule::getId()
{
    return id;
}

const  QMap<QString,QVariant> RobotModule::getSettingsCopy()
{
    QSettings s;
    s.beginGroup(id);

    QMap<QString,QVariant> map;

    QStringList keys = s.allKeys();

    foreach (QString key, keys) {
       map.insert(key,s.value(key));
    }
    return map;
}

QStringList RobotModule::getSettingKeys()
{
    QSettings s;
    s.beginGroup(id);
    QStringList list = s.allKeys();
    return list;
}


const QVariant RobotModule::getSettingsValue(const QString key, const QVariant defValue)
{
    QSettings s;
    s.beginGroup(id);
    QVariant qv = s.value(key,defValue);
    return qv;
}

const QVariant RobotModule::getSettingsValue(const QString key)
{
    QSettings s;
    s.beginGroup(id);
    QVariant qv = s.value(key);
    return qv;
}

void RobotModule::setSettingsValue(QString key, QVariant value)
{
    QSettings s;
    s.beginGroup(id);
    s.setValue(key,value);
}


void RobotModule::setDefaultValue(const QString &key, const QVariant &value)
{
    QSettings s;
    s.beginGroup(id);
    if (!s.contains(key))
        s.setValue(key, value);
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

bool RobotModule::shutdown()
{
    this->exit(0);
    this->wait();
    return true;
}

void RobotModule::reset()
{
    dataLockerMutex.lock();
    data.clear();
    dataLockerMutex.unlock();
    QMutexLocker l(&healthStatusMutex);
    healthStatus.errorCount=0;
    healthStatus.lastError="";
    healthStatus.healthOk=true;
}

void RobotModule::msleep(int millies)
{
    QThread::msleep(millies);
}
