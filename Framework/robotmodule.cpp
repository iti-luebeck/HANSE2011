#include "robotmodule.h"

RobotModule::RobotModule(QString newId)
    : QObject(), id(newId), settings(QSettings::IniFormat, QSettings::UserScope, "ITI", "Hanse")
{
    settings.beginGroup(id);
}

void RobotModule::enabled(bool value)
{
    settings.setValue("enabled", value);

    emit isEnabled(settings.value("enabled").toBool());
}


QString RobotModule::getTabName()
{
    return id;
}
