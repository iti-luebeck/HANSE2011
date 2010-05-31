#include "module_sonarlocalization.h"

#include <QtGui>

#include <Module_ScanningSonar/module_scanningsonar.h>

Module_SonarLocalization::Module_SonarLocalization(QString id, Module_ScanningSonar *sonar)
    : RobotModule(id)
{
    this->sonar = sonar;
}

void Module_SonarLocalization::reset()
{
    RobotModule::reset();
}

void Module_SonarLocalization::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_SonarLocalization::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sonar);
    return ret;
}

QWidget* Module_SonarLocalization::createView(QWidget* parent)
{
    return new QWidget(parent);
}

