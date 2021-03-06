#include "robotmodule.h"
#include "datarecorder.h"

#ifdef OS_UNIX
#include <sys/prctl.h>
#endif

#define HANSE_FRAMEWORK_CONFIG_DIR "HanseCfg"

RobotModule::RobotModule(QString newId)
    : dataLockerMutex(QMutex::Recursive),
      id(newId),
      healthCheckTimer(this),
      recorder(*this)
{
    // move this module and all its children (in the Qt "parent" hierarchy)
    // to the module's own thread
    moveToThread(this);

    this->initWaiterMutex.lock();

    // perform a health check once a second
    connect(&healthCheckTimer,SIGNAL(timeout()),this,SLOT(doHealthCheck()));
    healthCheckTimer.setInterval(1000);

    logger = Log4Qt::Logger::logger(id);
}

void RobotModule::run()
{
    logger->info("Started thread id="+QString::number((int)QThread::currentThreadId()));

    // start health check timer
    healthCheckTimer.start();

#ifdef OS_UNIX
    // linux allows to put names to threads. they can be seen in the process list.
    logger->trace("Setting pthread thread name");
    prctl(PR_SET_NAME,("H: "+id).toStdString().c_str(),0,0,0);
#endif

    logger->info("Initializing module");
    this->init();

    this->initWaiter.wakeAll();

    logger->info("Init complete, starting event loop");
    this->exec();

    logger->info("Event loop finished.");

    this->terminate();
    logger->info("Stopped thread");

}

void RobotModule::setEnabled(bool value)
{
    QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, "modulesgraph");
    s.setValue(id,value);

    if (value) {
        // this method may be called from other threads, therefore we do the reset
        // asynchronously
        QTimer::singleShot(0, this, SLOT(reset()));
    }
    emit enabled(isEnabled());

}

bool RobotModule::isEnabled()
{
    // "enabled" is not stored in the module-specific ini file, but in a special one
    // this avoids repeating SVN conflicts when commiting ini files.
    QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, "modulesgraph");
    return s.value(id).toBool();
}

void RobotModule::waitForInitToComplete()
{
    initWaiter.wait(&initWaiterMutex);
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
    QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, id);

    QMap<QString,QVariant> map;

    QStringList keys = s.allKeys();

    foreach (QString key, keys) {
       map.insert(key,s.value(key));
    }
    return map;
}

QStringList RobotModule::getSettingKeys()
{
    QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, id);
    QStringList list = s.allKeys();
    return list;
}


const QVariant RobotModule::getSettingsValue(const QString key, const QVariant defValue)
{
    if (key == "enabled") {
        return isEnabled();
    } else {
        QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, id);
        QVariant qv = s.value(key,defValue);
        return qv;
    }
}

const QVariant RobotModule::getSettingsValue(const QString key)
{
    if (key == "enabled") {
        return isEnabled();
    } else {
        QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, id);
        QVariant qv = s.value(key);
        return qv;
    }
}

void RobotModule::setSettingsValue(QString key, QVariant value)
{
    Q_ASSERT(key != "enabled");
    QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, id);
    s.setValue(key,value);
}


void RobotModule::setDefaultValue(const QString &key, const QVariant &value)
{
    Q_ASSERT(key != "enabled");
    QSettings s(QSettings::IniFormat, QSettings::UserScope,HANSE_FRAMEWORK_CONFIG_DIR, id);
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
    recorder.stopRecording();
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
