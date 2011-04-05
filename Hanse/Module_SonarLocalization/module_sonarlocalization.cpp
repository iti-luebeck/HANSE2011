#include "module_sonarlocalization.h"
#include <QtGui>
#include "form_sonarlocalization.h"
#include "sonarechofilter.h"
#include "sonarparticlefilter.h"
#include <opencv/cv.h>
#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <qfile.h>
#include <qtextstream.h>
#include <Module_ScanningSonar/sonardatasourcefile.h>

using namespace cv;

Module_SonarLocalization::Module_SonarLocalization(QString id, Module_ScanningSonar *sonar, Module_PressureSensor* pressure)
    : RobotModule(id),
        filter(this),
        pf(*this, filter)
{
       filter.moveToThread(this);
       pf.moveToThread(this);

       this->sonar = sonar;

}

void Module_SonarLocalization::init()
{
    logger->debug("INIT SONARLOC");

    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), &filter, SLOT(newSonarData(SonarReturnData)));
    qRegisterMetaType< QList<QVector2D> >("QList<QVector2D>");
    connect(&filter, SIGNAL(newImage(QList<QVector2D>)), &pf, SLOT(newImage(QList<QVector2D>)));
    connect(&pf, SIGNAL(newPosition(QVector3D)), this, SLOT(newPositionEst(QVector3D)));
    qRegisterMetaType< SonarEchoData > ("SonarEchoData");
    connect(&filter,SIGNAL(newSonarEchoData(SonarEchoData)),this,SLOT(retrieveSonarEchoData(SonarEchoData)));
}

void Module_SonarLocalization::reset()
{
    RobotModule::reset();
    filter.reset();
    pf.reset();
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

SonarParticleFilter& Module_SonarLocalization::particleFilter()
{
    return this->pf;
}

void Module_SonarLocalization::newPositionEst(QVector3D p)
{
    emit newLocalizationEstimate();
}

void Module_SonarLocalization::setLocalization(QVector2D position)
{
    QMutexLocker l(&this->dataLockerMutex);
    pf.setLocalization(position);
}

Position Module_SonarLocalization::getLocalization()
{
    this->dataLockerMutex.lock();
    QVector3D p = pf.getBestEstimate();
    this->dataLockerMutex.unlock();
    Position r;
    r.setX(p.x());
    r.setY(p.y());
//    if(pressure)
//    if(pressure && pressure->isEnabled() && pressure->getHealthStatus().isHealthOk())
//        r.setZ(pressure->getDepth());
    r.setZ(0);
    r.setYaw(p.z()*180/M_PI);
    return r;
}

float Module_SonarLocalization::getLocalizationConfidence()
{
    QMutexLocker l(&this->dataLockerMutex);
    return pf.getParticles()[0].w();
}

QDateTime Module_SonarLocalization::getLastRefreshTime()
{
    return QDateTime::currentDateTime(); // TODO
}

bool Module_SonarLocalization::isLocalizationLost()
{
    return false; // TODO must involve some kind of threshold
}

void Module_SonarLocalization::retrieveSonarEchoData(SonarEchoData data)
{
    emit newSonarEchoData(data);
}
