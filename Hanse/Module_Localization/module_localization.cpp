#include "module_localization.h"
#include "form_localization.h"
#include <QtGui>

#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>
#include <Module_PressureSensor/module_pressuresensor.h>

Module_Localization::Module_Localization(QString id, Module_VisualSLAM *visualSLAM, Module_SonarLocalization *sonarLoc, Module_PressureSensor *pressure)
    : RobotModule(id)
{
    this->visualSLAM = visualSLAM;
    this->sonarLocalization = sonarLoc;
    this->pressure = pressure;
}

void Module_Localization::reset()
{
    RobotModule::reset();
}

void Module_Localization::terminate()
{
    RobotModule::terminate();
}

QList<RobotModule*> Module_Localization::getDependencies()
{
    QList<RobotModule*> ret;
    ret.append(visualSLAM);
    ret.append(sonarLocalization);
    return ret;
}

QWidget* Module_Localization::createView(QWidget* parent)
{
    return new Form_Localization(parent);
}

void Module_Localization::doHealthCheck()
{

}