#include "module_sonarlocalization.h"

#include <QtGui>
#include "form_sonarlocalization.h"
#include "sonarechofilter.h"
#include "sonarparticlefilter.h"
#include <opencv/cv.h>
#include <Module_ScanningSonar/module_scanningsonar.h>

using namespace cv;

Module_SonarLocalization::Module_SonarLocalization(QString id, Module_ScanningSonar *sonar)
    : RobotModule(id)
{
    this->sonar = sonar;
    this->filter = new SonarEchoFilter(sonar);
    this->pf = new SonarParticleFilter(this, filter);
    pfThread.start();
    //pf->moveToThread(&pfThread);
}

void Module_SonarLocalization::reset()
{
    RobotModule::reset();
    // TODO
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
    return new Form_SonarLocalization(parent, this);
}

