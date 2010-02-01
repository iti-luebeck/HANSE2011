#include "robotmodule.h"

RobotModule::RobotModule(QString newId)
    : QObject(), id(newId), settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse")
{
    settings.beginGroup(id);
    setDefaultValue("enabled", true);
    logger = Log4Qt::Logger::logger(id);
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
