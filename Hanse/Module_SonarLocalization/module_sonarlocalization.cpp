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
    this->filter = new SonarEchoFilter(this);
    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), filter, SLOT(newSonarData(SonarReturnData)));

    // run particle filter in own thread
    this->pf = new SonarParticleFilter(this, filter);
    qRegisterMetaType< QList<QVector2D> >("QList<QVector2D>");
    connect(filter, SIGNAL(newImage(QList<QVector2D>)), pf, SLOT(newImage(QList<QVector2D>)));
    pfThread.start();
    pf->moveToThread(&pfThread);

    connect(pf, SIGNAL(newPosition(QVector3D)), this, SLOT(newPositionEst(QVector3D)));

}

void Module_SonarLocalization::reset()
{
    RobotModule::reset();
    filter->reset();
    pf->reset();
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

SonarParticleFilter& Module_SonarLocalization::particleFilter() const
{
    return *(this->pf);
}

void Module_SonarLocalization::newPositionEst(QVector3D p)
{
    emit newLocalizationEstimate();
}

void Module_SonarLocalization::setLocalization(QVector2D position)
{
    pf->setLocalization(position);
}
