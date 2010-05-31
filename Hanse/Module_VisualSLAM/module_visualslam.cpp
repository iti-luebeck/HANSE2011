#include "module_visualslam.h"

#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <QtGui>

Module_VisualSLAM::Module_VisualSLAM(QString id, Module_SonarLocalization *sonarLocalization)
    : RobotModule(id)
{
    this->sonarLocalization = sonarLocalization;
}

void Module_VisualSLAM::reset()
{
    RobotModule::reset();
}

void Module_VisualSLAM::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_VisualSLAM::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(sonarLocalization);
    return ret;
}

QWidget* Module_VisualSLAM::createView(QWidget* parent)
{
    return new QWidget(parent);
}
