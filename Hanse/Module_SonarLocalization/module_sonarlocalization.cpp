#include "module_sonarlocalization.h"
#include <QtCore>
#include "form_sonarlocalization.h"
#include <Module_ScanningSonar/module_scanningsonar.h>
#include <Module_PressureSensor/module_pressuresensor.h>
#include <Module_ScanningSonar/sonardatasourcefile.h>
#include <Module_XsensMTi/module_xsensmti.h>

using namespace cv;

Module_SonarLocalization::Module_SonarLocalization(QString id, Module_ScanningSonar *sonar, Module_XsensMTi *mti, Module_PressureSensor *pressure, Module_Simulation *sim)
    : RobotModule(id),
        filter(this, mti, sim),
        pf(*this, mti, filter)
{
    filter.moveToThread(this);
    pf.moveToThread(this);

    this->sonar = sonar;
    this->mti = mti;
    this->pressure = pressure;
}

void Module_SonarLocalization::init()
{
    logger->debug("INIT SONARLOC");
    qRegisterMetaType< QList<QVector2D> >("QList<QVector2D>");
    qRegisterMetaType< SonarEchoData > ("SonarEchoData");
    qRegisterMetaType< QList<SonarEchoData> > ("QList<SonarEchoData>");

    connect(sonar, SIGNAL(newSonarData(SonarReturnData)), &filter, SLOT(newSonarData(SonarReturnData)));
    connect(&filter, SIGNAL(newImage(QList<QVector2D>)), &pf, SLOT(newImage(QList<QVector2D>)));
    connect(&pf, SIGNAL(newPosition(QVector3D)), this, SIGNAL(newLocalizationEstimate()));
    connect(&filter,SIGNAL(newSonarEchoData(QList<SonarEchoData>)),this,SLOT(retrieveSonarEchoData(QList<SonarEchoData>)));
    connect(&filter,SIGNAL(newSonarPlotData(QList<SonarEchoData>)),this,SLOT(retrieveSonarPlotData(QList<SonarEchoData>)));

//    if (this->sonar->isEnabled() == false) {
//        this->sonar->setEnabled(true);
//    }

//    if (this->mti->isEnabled() == false) {
//        this->mti->setEnabled(true);
//    }

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
    r.setZ(pressure->getDepth());
    r.setYaw(p.z()*180/M_PI);
    return r;
}

float Module_SonarLocalization::getLocalizationConfidence()
{
    QMutexLocker l(&this->dataLockerMutex);
    QVector<SonarParticle> particles = pf.getParticles();
    return particles[0].getMatchRatio();
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

void Module_SonarLocalization::retrieveSonarPlotData(QList<SonarEchoData> data)
{
    emit newSonarPlotData(data);
}

void Module_SonarLocalization::particleFilterDone(QVector3D)
{
    emit dataChanged(this);
}

void Module_SonarLocalization::doHealthCheck() {
    if(!isEnabled()){
        setHealthToOk();
        return;
    }
    if (!pf.hasMap()) {
        setHealthToSick("No map loaded!");
    }
}
