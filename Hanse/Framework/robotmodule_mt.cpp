#include "Framework/robotmodule_mt.h"
#include "datarecorder.h"

RobotModule_MT::RobotModule_MT(QString id)
    : RobotModule(id)
{
    moduleThread.start(QThread::NormalPriority);
    this->moveToThread(&moduleThread);
    qDebug() << "THREAD ID ModuleMT";
    qDebug() << QThread::currentThreadId();
    qDebug() << "Module THREAD ID";
    qDebug() << moduleThread.currentThreadId();
}

const QMap<QString,QVariant> RobotModule_MT::getData()
{
    QMutexLocker l(&dataLockerMutex);
    QMap<QString,QVariant> map;
    QMap<QString, QVariant>::const_iterator i = data.constBegin();
    while (i != data.constEnd()) {
        map.insert(i.key(),i.value());
    }
    return map;
}


QVariant RobotModule_MT::getDataValue(QString key)
{
    QMutexLocker l(&dataLockerMutex);
    QVariant qv = data[key];
    return qv;
}

void RobotModule_MT::getDataValue(QString key, QVariant &data)
{
    QMutexLocker l(&dataLockerMutex);
    QVariant qv = this->data[key];
    data = qv;
}

void RobotModule_MT::addData(QString key, QVariant value)
{
    QMutexLocker l(&dataLockerMutex);
    data[key] = value;
}


QSettings& RobotModule_MT::getSettings()
{
    QMutexLocker l(&settingsMutex);
    settings.sync();
    return settings;
}

const QVariant RobotModule_MT::getSettingsValue(const QString key, const QVariant defValue)
{
    QMutexLocker l(&settingsMutex);
    QVariant qv = settings.value(key,defValue);
    return qv;
}

const QVariant RobotModule_MT::getSettingsValue(const QString key)
{
    QMutexLocker l(&settingsMutex);
    QVariant qv = settings.value(key);
    return qv;
}

void RobotModule_MT::getSettingsValueSl(QString key, QVariant &value)
{
    QMutexLocker l(&settingsMutex);
    QVariant qv = settings.value(key);
    value = qv;
}

void RobotModule_MT::getSettingsValueSl(QString key, QVariant defValue, QVariant &value)
{
    QMutexLocker l(&settingsMutex);
    QVariant qv = settings.value(key,defValue);
    value = qv;
}

void RobotModule_MT::setSettingsValue(QString key, QVariant value)
{
    QMutexLocker l(&settingsMutex);
    settings.setValue(key,value);
}

void RobotModule_MT::msleep(int millies)
{
    MyModuleThread::msleep(millies);
}

void RobotModule_MT::MyModuleThread::msleep(int millies)
{
    QThread::msleep(millies);
}

void RobotModule_MT::MyModuleThread::run()
{
    QThread::exec();

}
void RobotModule_MT::setDefaultValue(const QString &key, const QVariant &value)
{
    QMutexLocker l(&settingsMutex);
    if (!settings.contains(key))
        settings.setValue(key, value);
}
