#include "module_navigation.h"

#include <QtGui>
#include "form_navigation.h"

#include <Module_Localization/module_localization.h>
#include <Module_ThrusterControlLoop/module_thrustercontrolloop.h>

Module_Navigation::Module_Navigation(QString id, Module_Localization *localization, Module_ThrusterControlLoop *tcl)
    : RobotModule(id)
{
    this->localization = localization;
    this->tcl = tcl;
}

void Module_Navigation::reset()
{
    RobotModule::reset();
}

void Module_Navigation::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_Navigation::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(localization);
    ret.append(tcl);
    return ret;
}

QWidget* Module_Navigation::createView(QWidget* parent)
{
    return new Form_Navigation(parent);
}

void Module_Navigation::doHealthCheck()
{

}
