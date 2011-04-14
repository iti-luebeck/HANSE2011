#include "module_sonarlocalization.h"
#include <QtCore>
#include "form_sonarlocalization.h"
#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ScanningSonar/sonardatasourcefile.h>
#include <Module_XsensMTi/module_xsensmti.h>

using namespace cv;

Module_SonarLocalization::Module_SonarLocalization(QString id, Module_ScanningSonar *sonar, Module_XsensMTi *mti, Module_Simulation *sim)
    : RobotModule(id),
        filter(this, mti, sim),
        pf(*this, mti, filter)
{
    filter.moveToThread(this);
    pf.moveToThread(this);

    this->sonar = sonar;
    this->mti = mti;
}

void Module_SonarLocalization::init()
{
    logger->debug("INIT SONARLOC");

    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), &filter, SLOT(newSonarData(SonarReturnData)));
    qRegisterMetaType< QList<QVector2D> >("QList<QVector2D>");
    connect(&filter, SIGNAL(newImage(QList<QVector2D>)), &pf, SLOT(newImage(QList<QVector2D>)));
    connect(&pf, SIGNAL(newPosition(QVector3D)), this, SLOT(newPositionEst(QVector3D)));
    qRegisterMetaType< SonarEchoData > ("SonarEchoData");
    qRegisterMetaType< QList<SonarEchoData> > ("QList<SonarEchoData>");
    connect(&filter,SIGNAL(newSonarEchoData(QList<SonarEchoData>)),this,SLOT(retrieveSonarEchoData(QList<SonarEchoData>)));
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

SonarEchoFilter& Module_SonarLocalization::sonarEchoFilter()
{
    return this->filter;
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

void Module_SonarLocalization::retrieveSonarEchoData(QList<SonarEchoData> data)
{
    emit newSonarEchoData(data);
}
