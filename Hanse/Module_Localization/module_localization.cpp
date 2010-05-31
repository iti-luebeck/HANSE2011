#include "module_localization.h"
#include <QtGui>

#include <Module_VisualSLAM/module_visualslam.h>
#include <Module_SonarLocalization/module_sonarlocalization.h>

Module_Localization::Module_Localization(QString id, Module_VisualSLAM *visualSLAM, Module_SonarLocalization *sonarLoc)
    : RobotModule(id)
{
    this->visualSLAM = visualSLAM;
    this->sonarLocalization = sonarLoc;
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
    return new QWidget(parent);
}

void Module_Localization::doHealthCheck()
{

}
